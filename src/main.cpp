#include <Arduino.h>
#include <Wire.h>

#include <JC_Button.h>
#include <PrintEx.h>
#include <vector_math.h>

#include "calibrate.h"
#include "smoother.h"
#include "vector_math_ext.h"

#define LED0 13
#define LED1 12
#define BTN 11
#define JOYX A3
#define JOYY A2
#define JOYB 10

const int NUM_SAMPLES = 10;

using namespace vmath;

StreamEx eSerial = Serial;
MagCalibration magCali;

Button btn(BTN);
Button joyBtn(JOYB);

Smoother azi(NUM_SAMPLES);
Smoother alt(NUM_SAMPLES);
double lst = 0, lat = 0; // Can be changed with joy stick

// Altitude and azimuth to right accension and declination. All radians
void alazToRade(double alt, double azi, double lst, double lat, double &ra, double &dec) {
    /*
     * https://www.cloudynights.com/topic/448682-help-w-conversion-of-altaz-to-radec-for-dsc/?p=5811415
     *
     * sinD = sinA * sinL + cosA * cosL * cosAZ
     * cosH = (sinA - sinL * sinD) / (cosL * cosD)
     *
     * D=declination
     * H=hour angle
     * A=altitude
     * AZ=azimuth
     * L=latitude
     *
     * also, RA = LST - H
     */

    dec = asin(sin(alt) * sin(lat) + cos(alt) * cos(lat) * cos(azi));
    ra = fmod(lst - acos((sin(alt) - sin(lat) * sin(dec)) / (cos(lat) * cos(dec))), 2 * PI);
    if(ra < 0) ra += 2 * PI;
}

// Blocks until a :GD# or :GR# command is recieved
void lx200Comm(double ra, double dec) {
    char cmd0 = 0, cmd1 = 0;

    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);

    while(true) {
        while(!Serial.available());
        switch(char val = Serial.read()) {
            case '#':
                goto endLoop;
            case ':':
                cmd0 = cmd1 = 0;
                break;
            default:
                if(!cmd0)
                    cmd0 = val;
                else
                    cmd1 = val;
                break;
        }
    }
    endLoop:

    switch(cmd1) {
        case 'R':
            {
                long raSec = (long) (ra * 43200 / PI);
                eSerial.printf("%02ld:%02ld:%02ld#", raSec / 3600, raSec % 3600 / 60, raSec % 60);
                //Serial.prlong("03:00:00#");
                digitalWrite(LED0, HIGH);
                break;
            }
        case 'D':
            {
                long decSec = (long) abs(dec * 43200 / PI);
                eSerial.printf("%s%02ld*%02ld#", dec >= 0 ? "+" : "-", decSec / 3600, decSec % 3600 / 60);
                //Serial.print("+45*00#");
                digitalWrite(LED1, HIGH);
                break;
            }
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    // Setup MPU-6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0); // Disable power save
    Wire.endTransmission(true);

    // Setup HMC5883
    Wire.beginTransmission(0x1E);
    Wire.write(0x02);
    Wire.write(0x00); // Continuous measurement mode
    Wire.endTransmission();

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
    // Process input
    btn.read();
    joyBtn.read();

    if(btn.pressedFor(3000)) {
        // Calibrate magnetometer
        digitalWrite(LED0, HIGH);
        magCali = calibrateMag();
        digitalWrite(LED0, LOW);
    }

    int joyX = analogRead(JOYX);
    int joyY = analogRead(JOYY);
    if(joyX < 400)
        lst -= 0.1;
    else if(joyX > 624)
        lst += 0.1;
    if(joyY < 400)
        lat -= 0.1;
    else if(joyY > 624)
        lat += 0.1;

    // MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);

    vec3<double> acc;
    acc.x = Wire.read() << 8 | Wire.read();
    acc.y = Wire.read() << 8 | Wire.read();
    acc.z = Wire.read() << 8 | Wire.read();
    acc = normalize(acc); // Easier calculations

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
    mag *= magCali.scale; // Becomes normalized

    // Calculate altitude and azimuth
    auto forward = vec3<double> {0, 1, 0};
    auto north = project_to_plane(mag, acc);
    auto heading = project_to_plane(forward, acc);

    azi << angle_between(normalize(heading), normalize(north));
    alt << PI/2 - abs(angle_between(acc, forward)); // pi/2 - |angle to axis| = angle to plane prep. to axis

    // Calculate right accension and declination
    double ra = 0, dec = 0;
    alazToRade(alt, azi, lst, lat, ra, dec);

    // Communicate using lx200 protocol
    lx200Comm(ra, dec);

    //eSerial.printf("%3.2f, %3.2f\n", ra, dec);
    //long raSec = (long) (ra * 43200 / PI);
    //eSerial.printf("%02ld:%02ld:%02ld#\n", raSec / 3600, raSec % 3600 / 60, raSec % 60);
}
