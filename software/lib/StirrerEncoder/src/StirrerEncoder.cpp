#include "StirrerEncoder.h"

volatile int           StirrerEncoder::_isrSteps    = 0;
volatile unsigned long StirrerEncoder::_lastIsrTime = 0;

// ── ISR ──────────────────────────────────────────────────────
void IRAM_ATTR StirrerEncoder::isrEncoder() {
    unsigned long now = millis();
    if (now - _lastIsrTime < DEBOUNCE_MS) return;
    _lastIsrTime = now;
    bool clk = digitalRead(PIN_ENC_CLK);
    bool dt  = digitalRead(PIN_ENC_DT);
    _isrSteps += (clk != dt) ? 1 : -1;
}

// ── Setup ─────────────────────────────────────────────────────
void StirrerEncoder::setup() {
    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT,  INPUT_PULLUP);
    pinMode(PIN_ENC_SW,  INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), isrEncoder, CHANGE);
}

// ── Rotation ─────────────────────────────────────────────────
int StirrerEncoder::takeSteps() {
    if (_isrSteps == 0) return 0;
    noInterrupts();
    int steps = _isrSteps;
    _isrSteps = 0;
    interrupts();
    return steps;
}

// ── Button ───────────────────────────────────────────────────
EncoderPress StirrerEncoder::takePress() {
    bool          pressed = digitalRead(PIN_ENC_SW) == LOW;
    unsigned long now     = millis();

    if (pressed && !_btnPressed) {
        _btnPressed   = true;
        _btnPressTime = now;
    }
    if (!pressed && _btnPressed) {
        _btnPressed        = false;
        unsigned long held = now - _btnPressTime;
        if (held < BTN_DEBOUNCE_MS) return PRESS_NONE;
        return held >= LONGPRESS_MS ? PRESS_LONG : PRESS_SHORT;
    }
    return PRESS_NONE;
}
