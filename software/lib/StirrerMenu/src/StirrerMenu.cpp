#include "StirrerMenu.h"

// ── Setup ─────────────────────────────────────────────────────
void StirrerMenu::setup(StirrerConfig* config, StirrerControl* control,
                        StirrerEncoder* encoder, StirrerDisplay* display) {
    _config  = config;
    _control = control;
    _encoder = encoder;
    _display = display;

    _page        = PAGE_STATUS;
    _showMessage = false;
    _shownLine1[0] = '\0';
    _shownLine2[0] = '\0';
    _dirty = true;
    update();
}

// ── Update ────────────────────────────────────────────────────
void StirrerMenu::update() {
    unsigned long now = millis();

    int steps = _encoder->takeSteps();
    if (steps != 0) {
        int dir = (steps > 0) ? 1 : -1;
        for (int i = 0; i < abs(steps); i++) applyChange(dir);
        _dirty = true;
    }

    switch (_encoder->takePress()) {
        case PRESS_SHORT:
            _page  = (MenuPage)((_page + 1) % PAGE_COUNT);
            _dirty = true;
            break;
        case PRESS_LONG:
            _config->write();
            _showMessage  = true;
            _messageStart = now;
            break;
        case PRESS_NONE:
            break;
    }

    if (_showMessage) {
        if (now - _messageStart < MENU_MESSAGE_MS && !_dirty) {
            show("Config saved!", "");
            return;
        }
        // input ends the message early
        _showMessage = false;
        _dirty = true;
    }

    if (!_dirty && now - _lastRefresh < MENU_REFRESH_MS) return;
    _dirty       = false;
    _lastRefresh = now;

    constexpr int BUF = StirrerDisplay::LCD_COLS + 1;
    char line1[BUF], line2[BUF];
    render(line1, line2, BUF);
    show(line1, line2);
}

// ── Change value ─────────────────────────────────────────────
void StirrerMenu::applyChange(int dir) {
    switch (_page) {
        case PAGE_STATUS:
            break;
        case PAGE_TARGET_TEMP:
            _config->control.targetTemp =
                constrain(_config->control.targetTemp + dir * STEP_TEMP, 0, 350);
            break;
        case PAGE_TARGET_RPM:
            _config->control.targetRpm =
                constrain(_config->control.targetRpm + dir * STEP_RPM,
                          0, _config->control.maxRpm);
            break;
        case PAGE_TEMP_MAX:
            _config->control.tempPlateMax =
                constrain(_config->control.tempPlateMax + dir * STEP_TEMPMAX, 50, 350);
            break;
        case PAGE_SENSOR_MODE:
            _config->control.useExtSensor = (dir > 0);
            break;
        case PAGE_HEATER:
            _config->control.heaterEnabled = (dir > 0);
            break;
        case PAGE_MOTOR:
            _config->control.motorEnabled = (dir > 0);
            break;
        case PAGE_COUNT:
            break;
    }
}

// ── Display ──────────────────────────────────────────────────
void StirrerMenu::render(char* line1, char* line2, int size) {
    float tempPlate = _control->getTempPlate();
    float rpm       = _control->getCurrentRpm();

    switch (_page) {
        case PAGE_STATUS:
            snprintf(line1, size, "%5.1fC %5.0frpm",
                tempPlate, rpm);
            snprintf(line2, size, "%5.1fC %5.0frpm",
                _config->control.targetTemp,
                _config->control.targetRpm);
            break;
        case PAGE_TARGET_TEMP:
            snprintf(line1, size, "TEMP:  [%5.1fC]",
                _config->control.targetTemp);
            snprintf(line2, size, "Plate:  %5.1fC ",
                tempPlate);
            break;
        case PAGE_TARGET_RPM:
            snprintf(line1, size, "RPM:   [%5.0f] ",
                _config->control.targetRpm);
            snprintf(line2, size, "Ist: %7.0f rpm",
                rpm);
            break;
        case PAGE_TEMP_MAX:
            snprintf(line1, size, "MAX:   [%5.1fC]",
                _config->control.tempPlateMax);
            snprintf(line2, size, "Safety limit    ");
            break;
        case PAGE_SENSOR_MODE:
            snprintf(line1, size, "SENSOR:[%-5s]  ",
                _config->control.useExtSensor ? "EXT" : "PLATE");
            snprintf(line2, size, "Ext: %-11s",
                _control->isExtConnected() ? "connected" : "n/a");
            break;
        case PAGE_HEATER:
            snprintf(line1, size, "HEATER:  [%-3s]  ",
                _config->control.heaterEnabled ? "ON" : "OFF");
            snprintf(line2, size, "Plate:  %5.1fC ",
                tempPlate);
            break;
        case PAGE_MOTOR:
            snprintf(line1, size, "MOTOR:   [%-3s]  ",
                _config->control.motorEnabled ? "ON" : "OFF");
            snprintf(line2, size, "RPM: %9.0f  ",
                rpm);
            break;
        case PAGE_COUNT:
            line1[0] = '\0';
            line2[0] = '\0';
            break;
    }
}

void StirrerMenu::show(const char* line1, const char* line2) {
    if (strcmp(line1, _shownLine1) == 0 && strcmp(line2, _shownLine2) == 0) return;
    strncpy(_shownLine1, line1, sizeof(_shownLine1) - 1);
    strncpy(_shownLine2, line2, sizeof(_shownLine2) - 1);
    _shownLine1[sizeof(_shownLine1) - 1] = '\0';
    _shownLine2[sizeof(_shownLine2) - 1] = '\0';
    _display->write(line1, line2);
}
