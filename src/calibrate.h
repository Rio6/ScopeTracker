#pragma once

#include <vector_math.h>

struct MagCalibration {
    // Apply offset first, then scale
    vmath::vec3<double> offset; // Add this to get center
    vmath::vec3<double> scale; // Multiply this to get normalized strength
};

MagCalibration calibrateMag();
MagCalibration loadMagCalibration();
