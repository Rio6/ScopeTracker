#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>

#ifdef DEBUG
#include <printf.h>
#endif

#include "calibrate.h"

using namespace vmath;

extern const int LED0; // in main.cpp

template <int SIGN>
struct MaxList {
    static const size_t SIZE = 20;
    double nums[SIZE] = {0};

    void update(double val) {
        for(double &num : nums) {
            if(val * SIGN > num * SIGN) {
#ifdef DEBUG
                printf("Max %4.2f => %4.2f\n", num, val);
#endif
                num = val;
                break;
            }
        }
    }

    double avg() {
        double sum = 0;
        for(double num : nums)
            sum += num;
        return sum / SIZE;
    }
};

static struct MaxMin {
    double max = 0;
    double min = 0;

    bool update(double val) {
        if(val > max) max = val;
        else if(val < min) min = val;
        else return false;
        return true;
    }

    double offset() {
        return -(max + min) / 2;
    }

    double scale() {
        return 1 / (max - min);
    }
} x, y, z;

MagCalibration calibrateMag() {
    MagCalibration cali;
    unsigned long lastChanged = millis();
    while(millis() - lastChanged < 10000) {
        Wire.beginTransmission(0x1E);
        Wire.write(0x03);
        Wire.endTransmission();
        Wire.requestFrom(0x1E, 6, true);

        bool changed = false;
        changed |= y.update(Wire.read() << 8 | Wire.read()); // The board is rotated 90 deg
        changed |= z.update(Wire.read() << 8 | Wire.read());
        changed |= x.update(-(Wire.read() << 8 | Wire.read())); // so swap x y, invert x

        if(changed) {
            digitalWrite(LED0, HIGH);
            cali.offset = vec3<double> {x.offset(), y.offset(), z.offset()};
            cali.scale = vec3<double> {x.scale(), y.scale(), z.scale()};
#ifdef DEBUG
            printf("shift: % 3.2f % 3.2f % 3.2f scale: % 1.8f, % 1.8f, % 1.8f\n", cali.offset.x, cali.offset.y, cali.offset.z, cali.scale.x, cali.scale.y, cali.scale.z);
#endif
            lastChanged = millis();
        } else if(lastChanged - millis() > 100) {
            digitalWrite(LED0, LOW);
        }
    }

    EEPROM.put(0, cali);
    Serial.println("Magnetometer calibration saved");
    return cali;
}

MagCalibration loadMagCalibration() {
    MagCalibration cali;
    EEPROM.get(0, cali);
    return cali;
}
