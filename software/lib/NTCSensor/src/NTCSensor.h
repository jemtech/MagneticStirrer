#pragma once
#include <Arduino.h>

class NTCSensor {
public:
    NTCSensor(int pin, float rFixed = 22000.0, float rNominal = 100000.0,
              float tNominal = 25.0, float bCoeff = 3950.0);

    void  setup();
    float readTemp();
    bool  isConnected();

private:
    int   _pin;
    float _rFixed;
    float _rNominal;
    float _tNominal;
    float _bCoeff;
};
