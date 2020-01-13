#pragma once

template <typename T>
struct Smoother {
    int numSamples;
    int index = 0;
    T *vals;
    bool circular;

    Smoother(int numSamples, bool circular = false)
        : numSamples(numSamples), circular(circular) {
        vals = new T[numSamples];
    }

    Smoother(const Smoother &other) : Smoother(other.numSamples, other.circular) {
        memcpy(vals, other.vals, numSamples * sizeof(T));
    }

    Smoother &operator=(const Smoother&) = delete;

    ~Smoother() {
        delete[] vals;
    }

    void update(T val) {
        vals[index] = val;
        ++index %= numSamples;
    }

    T getValue() {
        if(circular) {
            T avgSin = 0, avgCos = 0;
            for(int i = 0; i < numSamples; i++) {
                avgSin += sin(vals[i]) / numSamples;
                avgCos += cos(vals[i]) / numSamples;
            }
            return avgCos == 0 && avgSin == 0 ? 0 : atan2(avgSin, avgCos);
        } else {
            T avg = 0;
            for(int i = 0; i < numSamples; i++)
                avg += vals[i] / numSamples;
            return avg;
        }
    }

    Smoother &operator<<(T val) {
        update(val);
        return *this;
    }

    operator T() {
        return getValue();
    }
};
