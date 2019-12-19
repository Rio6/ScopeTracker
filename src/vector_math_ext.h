#pragma once

#include <vector_math.h>

namespace vmath {
    template <typename T>
    T sign(T num) {
        return (num > 0) - (num < 0);
    }

    template <typename T>
    void try_normalize(vec3<T> &v) {
        if(abs(dot(v, v) - 1.0) > 1e-6) v = normalize(v);
    }

    template<typename T>
    inline vec3<T> project_to_plane(vec3<T> a, vec3<T> norm) {
        try_normalize(norm);
        return a - dot(a, norm) * norm;
    }

    template <typename T>
    inline T angle_between(vec3<T> a, vec3<T> b) {
        try_normalize(a);
        try_normalize(b);
        double s = sign(cross(a, b).z); // TODO get sign of the angle properly
        return s * acos(dot(a, b));
    }

    template <typename T>
    inline quat<T> gyro_to_quat(vec3<T> omega) {
        T theta = length(omega);
        auto v = omega / theta; // normalized vector
        T s = sin(theta/2), c = cos(theta/2);
        return quat<T> (s * v.x, s * v.y, s * v.z, c);
    }

    template <typename T>
    inline T project_quat(quat<T> rot, vec3<T> v) {
        auto prep = cross(v, rot.v); // a vector prependicular to v
        auto rotated = quat_to_mat3(rot) * prep;
        return angle_between(project_to_plane(rotated, v), prep);
    }
};
