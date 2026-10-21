#pragma once
#include <Arduino.h>
#include "StirrerConfig.h"
#include "StirrerDisplay.h"

// ── Pin-Definitionen ─────────────────────────────────────────
#define PIN_ENC_CLK   32
#define PIN_ENC_DT    33
#define PIN_ENC_SW    14

// ── Timing ───────────────────────────────────────────────────
#define DEBOUNCE_MS      5
#define BTN_DEBOUNCE_MS  50
#define LONGPRESS_MS     800

// ── Schrittweiten ────────────────────────────────────────────
#define STEP_TEMP     1.0f
#define STEP_RPM      10
#define STEP_TEMPMAX  5.0f

enum EncoderParam {
    PARAM_STATUS = 0,
    PARAM_TARGET_TEMP,
    PARAM_TARGET_RPM,
    PARAM_TEMP_MAX,
    PARAM_SENSOR_MODE,
    PARAM_HEATER,
    PARAM_MOTOR,
    PARAM_COUNT
};

class StirrerEncoder {
public:
    void setup(StirrerConfig* config, StirrerDisplay* display);
    void update();
    void updateSensorValues(float tempPlate, float tempExt,
                            bool extConnected, float rpm);

private:
    // FIX #1: ISR-Variablen sind private (statische private Member
    //         sind für statische private Methoden zugänglich)
    static volatile int           _isrSteps;
    static volatile unsigned long _lastIsrTime;  // FIX #3: unsigned long

    static void IRAM_ATTR isrEncoder();

    void handleSteps();
    void handleButton();
    void applyChange(int dir);
    void onShortPress();
    void onLongPress();
    void updateDisplay();

    StirrerConfig*  _config        = nullptr;
    StirrerDisplay* _display       = nullptr;
    EncoderParam    _currentParam  = PARAM_STATUS;
    bool            _btnPressed    = false;
    unsigned long   _btnPressTime  = 0;   // FIX #3: unsigned long
    float           _lastTempPlate = 0;
    float           _lastTempExt   = 0;
    float           _lastRpm       = 0;
    bool            _extConnected  = false;
};
