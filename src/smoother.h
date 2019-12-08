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

    Smoother &operator=(const Smoother &other) {
        memcpy(vals, other.vals, numSamples * sizeof(double));
        return *this;
    }

    ~Smoother() {
        delete[] vals;
    }

    void update(double val) {
        vals[index] = val;
        ++index %= numSamples;
    }

    double getValue() {
        double sum = 0;
        for(int i = 0; i < numSamples; i++)
            sum += vals[i];
        return sum / numSamples;
    }

    Smoother &operator<<(double val) {
        update(val);
        return *this;
    }

    operator double() {
        return getValue();
    }
};
