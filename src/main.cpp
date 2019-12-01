#include <Arduino.h>
#include <Wire.h>

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

    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(11, INPUT_PULLUP);
}

void loop() {
    Wire.beginTransmission(0x68);
    Wire.write(0x3B);
    Wire.endTransmission(false);

    Wire.requestFrom(0x68, 14, true);
    double accX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    double accY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    double accZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
    double temp = Wire.read() << 8 | Wire.read();
    double gyroX = Wire.read() << 8 | Wire.read();
    double gyroY = Wire.read() << 8 | Wire.read();
    double gyroZ = Wire.read() << 8 | Wire.read();

    Wire.beginTransmission(0x1E);
    Wire.write(0x03);
    Wire.endTransmission();

    Wire.requestFrom(0x1E, 6, true);
    double  magX = Wire.read() << 8 | Wire.read();
    double  magZ = Wire.read() << 8 | Wire.read();
    double  magY = Wire.read() << 8 | Wire.read();

    double pitch = atan2(accY, sqrt(accX*accX + accZ*accZ));
    double heading = atan2(magX * accZ - magZ * accY, magY); // The board is sideways

    Serial.print(pitch); Serial.print(" "); Serial.println(heading);

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
