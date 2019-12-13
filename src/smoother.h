#pragma once

struct Smoother {
    int numSamples;
    int index = 0;
    double *vals;

    Smoother(int numSamples) : numSamples(numSamples) {
        vals = new double[numSamples];
    }

    Smoother(const Smoother &other) : Smoother(other.numSamples) {
        memcpy(vals, other.vals, numSamples * sizeof(double));
    }

    Smoother &operator=(const Smoother&) = delete;

    ~Smoother() {
        delete[] vals;
    }

    void update(double val) {
        vals[index] = val;
        ++index %= numSamples;
    }

    double getValue() {
        double avg = 0;
        for(int i = 0; i < numSamples; i++)
            avg += vals[i] / numSamples;
        return avg;
    }

    Smoother &operator<<(double val) {
        update(val);
        return *this;
    }

    operator double() {
        return getValue();
    }
};
