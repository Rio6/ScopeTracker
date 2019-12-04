#include <Arduino.h>
#include <Wire.h>

#include <MadgwickAHRS.h>
#include <PrintEx.h>
#include <vector_math.h>

using namespace vmath;

StreamEx eSerial = Serial;

void setup() {
    Serial.begin(9600);
    Wire.begin();

    // Setup MPU-6050
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    // Setup HMC5883
    Wire.beginTransmission(0x1E);
    Wire.write(0x02);
    Wire.write(0x00); // Continuous measurement mode
    Wire.endTransmission();

    // Joystick
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(10, INPUT_PULLUP);

    digitalWrite(3, LOW); // gnd
    digitalWrite(4, HIGH); // v+

    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(11, INPUT_PULLUP);
}

void loop() {
    // MPU6050
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(0x68, 14, true);

    vec3<double> acc;
    acc.x = (Wire.read() << 8 | Wire.read()) / 16384.0;
    acc.y = (Wire.read() << 8 | Wire.read()) / 16384.0;
    acc.z = (Wire.read() << 8 | Wire.read()) / 16384.0;

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
    mag.y = (Wire.read() << 8 | Wire.read()) / 1370.0; // The board is rotated 90 deg
    mag.z = (Wire.read() << 8 | Wire.read()) / 1370.0;
    mag.x =-(Wire.read() << 8 | Wire.read()) / 1370.0; // so swap x y, invert x

    double alt = asin(dot(acc, vec3<double> {0, 1, 0}) / length(acc));

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
