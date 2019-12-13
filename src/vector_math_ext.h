#pragma once

#include <vector_math.h>

namespace vmath {
    template<typename T>
    inline vec3<T> project_to_plane(vec3<T> a, vec3<T> norm) {
        return a - dot(a, norm) * norm;
    }

    template<typename T>
    inline T angle_between(vec3<T> a, vec3<T> b) {
        return acos(dot(a, b) / (length(a) * length(b)));
    }

    template<typename T>
    inline T angle_between_normalized(vec3<T> a, vec3<T> b) {
        return acos(dot(a, b));
    }

    template<typename T>
    inline quat<T> euler_to_quat(T yaw, T pitch, T roll) {
        T cy = cos(yaw / T(2));
        T sy = sin(yaw / T(2));
        T cp = cos(pitch / T(2));
        T sp = sin(pitch / T(2));
        T cr = cos(roll / T(2));
        T sr = sin(roll / T(2));

        return quat<T> {
            cy * cp * sr - sy * sp * cr,
            sy * cp * sr + cy * sp * cr,
            sy * cp * cr - cy * sp * sr,
            cy * cp * cr + sy * sp * sr
        };
    }
};
