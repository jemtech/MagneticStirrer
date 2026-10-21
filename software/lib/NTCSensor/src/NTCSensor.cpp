#include "NTCSensor.h"
#include <math.h>

NTCSensor::NTCSensor(int pin, float rFixed, float rNominal,
                     float tNominal, float bCoeff)
    : _pin(pin), _rFixed(rFixed), _rNominal(rNominal),
      _tNominal(tNominal), _bCoeff(bCoeff) {}

void NTCSensor::setup() {
    pinMode(_pin, INPUT);
}

float NTCSensor::readTemp() {
    // Average over 8 readings to reduce ADC noise
    int raw = 0;
    for (int i = 0; i < 8; i++) raw += analogRead(_pin);
    raw /= 8;

    float voltage = (raw / 4095.0f) * 3.3f;
    if (voltage <= 0 || voltage >= 3.3f) return -999.0f;

    float rNtc = _rFixed * voltage / (3.3f - voltage);

    // Steinhart-Hart B-coefficient approximation
    float steinhart = rNtc / _rNominal;
    steinhart = log(steinhart);
    steinhart /= _bCoeff;
    steinhart += 1.0f / (_tNominal + 273.15f);
    steinhart = 1.0f / steinhart;
    steinhart -= 273.15f;

    return steinhart;
}

bool NTCSensor::isConnected() {
    int raw = analogRead(_pin);
    return (raw > 100 && raw < 3990);
}
