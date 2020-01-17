#include <Arduino.h>
#include <Wire.h>

#include <CoordsLib.h>
#include <printf.h>
#include <JC_Button.h>
#include <vector_math.h>

#include "calibrate.h"
#include "smoother.h"
#include "vector_math_ext.h"
#include "printf.h"

// Pins
extern const int LED0 = 13;
extern const int LED1 = 12;
extern const int BTN  = 11;
extern const int JOYX = A3;
extern const int JOYY = A2;
extern const int JOYB = 10;

const int NUM_SAMPLES = 50;

const double JOY_THRES = 0.1;
const double JOY_SPEED_FAST = 2*PI / 500;
const double JOY_SPEED_SLOW = 2*PI / 3000;

using namespace vmath;

MagCalibration magCali;
CoordsLib coords;

Button btn(BTN);
Button joyBtn(JOYB);

double ra = 0, dec = 0;
Smoother<double> azi(NUM_SAMPLES, true);
Smoother<double> alt(NUM_SAMPLES, true);

bool update_ref = false;
int refCount = 0;

void lx200Comm(double *ra, double *dec, bool update=false) {
    static bool process = false;

    if(!Serial.available()) return;

    if(!process) {
        if(Serial.read() != ':') {
            process = true;
        }
        return;
    }

    switch(Serial.read()) {
        case ':':
            process = false;
            break;
        case 'R': // get ra
            {
                long raSec = (long) (*ra * 43200 / PI);
                printf("%02ld:%02ld:%02ld#", raSec / 3600, raSec % 3600 / 60, raSec % 60);
                break;
            }
        case 'D': // get dec
            {
                long decSec = (long) abs(*dec * 648000 / PI);
                printf("%s%02ld*%02ld#", dec >= 0 ? "+" : "-", decSec / 3600, decSec % 3600 / 60);
                break;
            }
        case 'r': // set ra
            if(update) {
                const int LEN = 8; // HH:MM:SS
                char msg[LEN+1] = {0};
                Serial.readBytes(msg, LEN);

                int h=0, m=0, s=0;
                if(sscanf(msg, "%d:%d:%d", &h, &m, &s) >= 3) {
                    *ra = (h * 3600 + m * 60 + s) / 43200.0 * PI;
                    printf("1");
                    break;
                }
            }

            printf("0");
            break;

        case 'd': // set dec
            if(update) {
                const int LEN = 6; // sDD*MM
                char msg[LEN+1] = {0};
                Serial.readBytes(msg, LEN);

                int deg=0, m=0;
                if(sscanf(msg, "%d*%d", &deg, &m) >= 2) {
                    *dec = (deg + m / 60.0 * sign(deg)) / 180.0 * PI;
                    printf("1");
                    break;
                }
            }

            printf("0");
            break;

        case 'M': // sync
            update_ref = true;
        case 'S': // slew
            printf("1#");
            break;
    }
}

void _putchar(char c) { // for printf
    Serial.print(c);
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    // Setup MPU-6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0x00); // Disable power save
    Wire.endTransmission(true);

    // Setup HMC5883
    Wire.beginTransmission(0x1E);
    Wire.write(0x02);
    Wire.write(0x00); // Continuous measurement mode
    Wire.endTransmission();

    // Set up CoordsLib
    coords.setTime(millis() / 1000.0);

    // Joystick
    pinMode(JOYX, INPUT);
    pinMode(JOYY, INPUT);
    joyBtn.begin();

    // Buttons and leds
    pinMode(LED0, OUTPUT);
    pinMode(LED1, OUTPUT);
    btn.begin();

    // Load saved magnetometer calibration
    magCali = loadMagCalibration();
}

void loop() {
    double t = millis() / 1000;

    // MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);

    vec3<double> acc;
    acc.x = Wire.read() << 8 | Wire.read();
    acc.y = Wire.read() << 8 | Wire.read();
    acc.z = Wire.read() << 8 | Wire.read();

    double temp = ((Wire.read() << 8 | Wire.read()) + 521) / 340.0;

    vec3<double> gyro;
    gyro.x = (Wire.read() << 8 | Wire.read()) / 1090.0;
    gyro.y = (Wire.read() << 8 | Wire.read()) / 1090.0;
    gyro.z = (Wire.read() << 8 | Wire.read()) / 1090.0;

    // HMC5883
    Wire.beginTransmission(0x1E);
    Wire.write(0x03);
    Wire.endTransmission();
    Wire.requestFrom(0x1E, 6, true);

    vec3<double> mag;
    mag.y = (Wire.read() << 8 | Wire.read()); // The board is rotated 90 deg
    mag.z = (Wire.read() << 8 | Wire.read());
    mag.x =-(Wire.read() << 8 | Wire.read()); // so swap x y, invert x

    // Apply calibration
    mag += magCali.offset;
    mag *= magCali.scale;

    // Easier calculations
    acc = normalize(acc);
    gyro = gyro / 131.0 * 180.0 / PI; // radians/s
    mag = normalize(mag);

    // Calculate altitude and azimuth
    auto forward = vec3<double> {0, 1, 0};
    auto north = project_to_plane(mag, acc);
    auto heading = project_to_plane(forward, acc);

    azi << angle_between(heading, north) * sign(cross(heading, north).z);
    alt << PI/2 - abs(angle_between(acc, forward)); // pi/2 - |angle to axis| = angle to plane prep. to axis

    // Human input
    btn.read();
    joyBtn.read();

    // Magnetometer calibration
    if(btn.pressedFor(2000)) {
        magCali = calibrateMag();
        digitalWrite(LED0, HIGH);
        delay(500); // Lazy way to make led more visible
        digitalWrite(LED0, LOW);
    }

    // Get right accension and declination

    if(refCount < 3) {
        // Aligning. Use joystick to change ra and dec to match the scope
        static bool fastMove = true;

        double joyX = analogRead(JOYX) / 1024.0 - 0.5;
        double joyY = analogRead(JOYY) / 1024.0 - 0.5;

        if(joyBtn.wasPressed())
            fastMove = !fastMove;

        if(btn.wasPressed())
            update_ref = true;

        if(joyX < -JOY_THRES || joyX > JOY_THRES)
            ra += joyX * (fastMove ? JOY_SPEED_FAST : JOY_SPEED_SLOW);
        if(joyY < -JOY_THRES || joyY > JOY_THRES)
            dec += joyY * (fastMove ? JOY_SPEED_FAST : JOY_SPEED_SLOW);

        if(update_ref) { // Set the reference
            switch(refCount) {
                case 0: coords.setRef_1(ra, dec, t, azi, alt); break;
                case 1: coords.setRef_2(ra, dec, t, azi, alt); break;
                case 2: coords.setRef_3(ra, dec, t, azi, alt); break;
            }
            update_ref = false;
            refCount++;
        }

        // Show current reference count using LED
        if((refCount+1) & 0b01) digitalWrite(LED0, HIGH);
        else                  digitalWrite(LED0, LOW);
        if((refCount+1) & 0b10) digitalWrite(LED1, HIGH);
        else                  digitalWrite(LED1, LOW);

    } else if(btn.wasPressed()) {
        // Start aligning
        refCount = 0;
        ra = dec = 0;

    } else {
        // Calculate right accension and declination
        coords.getECoords(azi, alt, t, &ra, &dec);
    }

    ra = wrap(ra, 0.0, 2*PI);
    dec = wrap(dec, -PI/2, PI/2);

    // Communicate using lx200 protocol
#ifndef DEBUG
    lx200Comm(&ra, &dec, refCount < 3);
#else
    printf("azi %lf alt %lf ra %lf dec %lf\n", azi.getValue(), alt.getValue(), ra, dec);
#endif
}
