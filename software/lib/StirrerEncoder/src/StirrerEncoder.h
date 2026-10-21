#pragma once
#include <Arduino.h>

// ── Pin definitions ───────────────────────────────────────────
#define PIN_ENC_CLK   32
#define PIN_ENC_DT    33
#define PIN_ENC_SW    14

// ── Timing ───────────────────────────────────────────────────
#define DEBOUNCE_MS      5
#define BTN_DEBOUNCE_MS  50
#define LONGPRESS_MS     800

enum EncoderPress {
    PRESS_NONE = 0,
    PRESS_SHORT,
    PRESS_LONG
};

// Push and turn input only, knows nothing about config or display.
class StirrerEncoder {
public:
    void setup();
    // steps turned since the last call, positive = clockwise
    int takeSteps();
    // polls the button, reports a press once it was released
    EncoderPress takePress();

private:
    // ISR state, private members are accessible from the static ISR
    static volatile int           _isrSteps;
    static volatile unsigned long _lastIsrTime;

    static void IRAM_ATTR isrEncoder();

    bool          _btnPressed   = false;
    unsigned long _btnPressTime = 0;
};
