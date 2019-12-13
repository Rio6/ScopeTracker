#pragma once

struct Smoother {
    int numSamples;
    int index = 0;
    double *vals;
    bool circular;

    Smoother(int numSamples, bool circular = false)
        : numSamples(numSamples), circular(circular) {
        vals = new double[numSamples];
    }

    Smoother(const Smoother &other) : Smoother(other.numSamples, other.circular) {
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
        if(circular) {
            double avgSin = 0, avgCos = 0;
            for(int i = 0; i < numSamples; i++) {
                avgSin += sin(vals[i]) / numSamples;
                avgCos += cos(vals[i]) / numSamples;
            }
            return avgCos == 0 && avgSin == 0 ? 0 : atan2(avgSin, avgCos);
        } else {
            double avg = 0;
            for(int i = 0; i < numSamples; i++)
                avg += vals[i] / numSamples;
            return avg;
        }
    }

    Smoother &operator<<(double val) {
        update(val);
        return *this;
    }

    operator double() {
        return getValue();
    }
};
