#pragma once
#include <Arduino.h>

// ── Pin-Definitionen ─────────────────────────────────────────
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
    // FIX #1: ISR-Variablen sind private (statische private Member
    //         sind für statische private Methoden zugänglich)
    static volatile int           _isrSteps;
    static volatile unsigned long _lastIsrTime;  // FIX #3: unsigned long

    static void IRAM_ATTR isrEncoder();

    bool          _btnPressed   = false;
    unsigned long _btnPressTime = 0;   // FIX #3: unsigned long
};
