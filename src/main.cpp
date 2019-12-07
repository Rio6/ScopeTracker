#include <Arduino.h>
#include <Wire.h>

#include <JC_Button.h>
#include <PrintEx.h>
#include <vector_math.h>

#include "calibrate.h"

#define LED0 13
#define LED1 12
#define BTN 11
#define JOYX A2
#define JOYY A3
#define JOYB 10

using namespace vmath;

StreamEx eSerial = Serial;
Button btn(BTN);
Button joyBtn(JOYB);
MagCalibration magCali;

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
        magCali = calibrateMag();
    }

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

    auto forward = vec3<double> {0, 1, 0};

    double alt = asin(dot(acc, forward)); // angle between acc vector and xz plane

    auto north = mag - dot(mag, acc) * acc; // project to plane prependicular to acc
    auto head = forward - dot(forward, acc) * acc; // ^
    double heading = acos(dot(head, normalize(north)) / length(head));

    eSerial.printf("%2.2f: %2.2f, %2.2f, %2.2f\n", heading, mag.x, mag.y, mag.z);

    /*
    static char cmd0 = 0, cmd1 = 0;
    bool finished = false;
    while(!finished && Serial.available()) {
        switch(char val = Serial.read()) {
            case '#':
                finished = true;
                break;
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

    if(finished) {
        switch(cmd1) {
            case 'R':
                Serial.print("03:00:00#");
                digitalWrite(13, HIGH);
                break;
            case 'D':
                Serial.print("+45*00#");
                digitalWrite(12, HIGH);
                break;
        }
        cmd0 = cmd1 = 0;
    } else {
        digitalWrite(12, LOW);
        digitalWrite(13, LOW);
    }
    */

    delay(100);
}
