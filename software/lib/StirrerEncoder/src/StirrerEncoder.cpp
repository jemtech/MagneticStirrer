#include "StirrerEncoder.h"

volatile int           StirrerEncoder::_isrSteps    = 0;
volatile unsigned long StirrerEncoder::_lastIsrTime = 0;  // FIX #3: unsigned long

// ── ISR ──────────────────────────────────────────────────────
void IRAM_ATTR StirrerEncoder::isrEncoder() {
    unsigned long now = millis();  // FIX #3: unsigned long
    if (now - _lastIsrTime < DEBOUNCE_MS) return;
    _lastIsrTime = now;
    bool clk = digitalRead(PIN_ENC_CLK);
    bool dt  = digitalRead(PIN_ENC_DT);
    _isrSteps += (clk != dt) ? 1 : -1;
}

// ── Setup ─────────────────────────────────────────────────────
void StirrerEncoder::setup(StirrerConfig* config, StirrerDisplay* display) {
    _config  = config;
    _display = display;

    pinMode(PIN_ENC_CLK, INPUT_PULLUP);
    pinMode(PIN_ENC_DT,  INPUT_PULLUP);
    pinMode(PIN_ENC_SW,  INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_ENC_CLK), isrEncoder, CHANGE);

    _currentParam = PARAM_STATUS;
    updateDisplay();
}

// ── Update ────────────────────────────────────────────────────
void StirrerEncoder::update() {
    handleSteps();
    handleButton();
}

void StirrerEncoder::updateSensorValues(float tempPlate, float tempExt,
                                        bool extConnected, float rpm) {
    _lastTempPlate = tempPlate;
    _lastTempExt   = tempExt;
    _extConnected  = extConnected;
    _lastRpm       = rpm;
    updateDisplay();
}

// ── Rotation ─────────────────────────────────────────────────
void StirrerEncoder::handleSteps() {
    if (_isrSteps == 0) return;
    noInterrupts();
    int steps = _isrSteps;
    _isrSteps = 0;
    interrupts();
    int dir   = (steps > 0) ? 1 : -1;
    int count = abs(steps);
    for (int i = 0; i < count; i++) applyChange(dir);
    updateDisplay();
}

void StirrerEncoder::applyChange(int dir) {
    switch (_currentParam) {
        case PARAM_STATUS:
            break;
        case PARAM_TARGET_TEMP:
            _config->control.targetTemp =
                constrain(_config->control.targetTemp + dir * STEP_TEMP, 0, 350);
            break;
        case PARAM_TARGET_RPM:
            _config->control.targetRpm =
                constrain(_config->control.targetRpm + dir * STEP_RPM,
                          0, _config->control.maxRpm);
            break;
        case PARAM_TEMP_MAX:
            _config->control.tempPlateMax =
                constrain(_config->control.tempPlateMax + dir * STEP_TEMPMAX, 50, 350);
            break;
        case PARAM_SENSOR_MODE:
            _config->control.useExtSensor = (dir > 0);
            break;
        case PARAM_HEATER:
            _config->control.heaterEnabled = (dir > 0);
            break;
        case PARAM_MOTOR:
            _config->control.motorEnabled = (dir > 0);
            break;
    }
}

// ── Button ───────────────────────────────────────────────────
void StirrerEncoder::handleButton() {
    bool          pressed = digitalRead(PIN_ENC_SW) == LOW;
    unsigned long now     = millis();  // FIX #3: unsigned long

    if (pressed && !_btnPressed) {
        _btnPressed   = true;
        _btnPressTime = now;
    }
    if (!pressed && _btnPressed) {
        _btnPressed        = false;
        unsigned long held = now - _btnPressTime;  // FIX #3: unsigned long
        if (held < BTN_DEBOUNCE_MS) return;
        if (held >= LONGPRESS_MS) {
            onLongPress();
        } else {
            onShortPress();
        }
    }
}

void StirrerEncoder::onShortPress() {
    _currentParam = (EncoderParam)((_currentParam + 1) % PARAM_COUNT);
    updateDisplay();
}

void StirrerEncoder::onLongPress() {
    _config->write();
    _display->write("Config saved!", "");
    delay(800);
    updateDisplay();
}

// ── Display ──────────────────────────────────────────────────
void StirrerEncoder::updateDisplay() {
    // FIX #10: BUF statt Magic Number 17
    constexpr int BUF = 17;  // LCD_COLS (16) + 1
    char line1[BUF], line2[BUF];

    switch (_currentParam) {
        case PARAM_STATUS:
            snprintf(line1, BUF, "%5.1fC %5.0frpm",
                _lastTempPlate, _lastRpm);
            snprintf(line2, BUF, "%5.1fC %5.0frpm",
                _config->control.targetTemp,
                _config->control.targetRpm);
            break;
        case PARAM_TARGET_TEMP:
            snprintf(line1, BUF, "TEMP:  [%5.1fC]",
                _config->control.targetTemp);
            snprintf(line2, BUF, "Plate:  %5.1fC ",
                _lastTempPlate);
            break;
        case PARAM_TARGET_RPM:
            snprintf(line1, BUF, "RPM:   [%5.0f] ",
                _config->control.targetRpm);
            snprintf(line2, BUF, "Ist: %7.0f rpm",
                _lastRpm);
            break;
        case PARAM_TEMP_MAX:
            snprintf(line1, BUF, "MAX:   [%5.1fC]",
                _config->control.tempPlateMax);
            snprintf(line2, BUF, "Safety limit    ");
            break;
        case PARAM_SENSOR_MODE:
            snprintf(line1, BUF, "SENSOR:[%-5s]  ",
                _config->control.useExtSensor ? "EXT" : "PLATE");
            snprintf(line2, BUF, "Ext: %-11s",
                _extConnected ? "connected" : "n/a");
            break;
        case PARAM_HEATER:
            snprintf(line1, BUF, "HEATER:  [%-3s]  ",
                _config->control.heaterEnabled ? "ON" : "OFF");
            snprintf(line2, BUF, "Plate:  %5.1fC ",
                _lastTempPlate);
            break;
        case PARAM_MOTOR:
            snprintf(line1, BUF, "MOTOR:   [%-3s]  ",
                _config->control.motorEnabled ? "ON" : "OFF");
            snprintf(line2, BUF, "RPM: %9.0f  ",
                _lastRpm);
            break;
    }
    _display->write(line1, line2);
}
