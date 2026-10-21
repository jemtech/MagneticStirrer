#pragma once
#include <Arduino.h>
#include "StirrerConfig.h"
#include "StirrerControl.h"
#include "StirrerDisplay.h"
#include "StirrerEncoder.h"

// ── Timing ───────────────────────────────────────────────────
#define MENU_REFRESH_MS   250   // redraw interval for changing sensor values
#define MENU_MESSAGE_MS   800   // how long "Config saved!" is shown

// ── Schrittweiten ────────────────────────────────────────────
#define STEP_TEMP     1.0f
#define STEP_RPM      10
#define STEP_TEMPMAX  5.0f

enum MenuPage {
    PAGE_STATUS = 0,
    PAGE_TARGET_TEMP,
    PAGE_TARGET_RPM,
    PAGE_TEMP_MAX,
    PAGE_SENSOR_MODE,
    PAGE_HEATER,
    PAGE_MOTOR,
    PAGE_COUNT
};

// Menu on the display, operated with the encoder:
// short press = next page, turn = change value, long press = save config.
class StirrerMenu {
public:
    void setup(StirrerConfig* config, StirrerControl* control,
               StirrerEncoder* encoder, StirrerDisplay* display);
    void update();

private:
    void applyChange(int dir);
    void render(char* line1, char* line2, int size);
    // writes to the display only when the text changed
    void show(const char* line1, const char* line2);

    StirrerConfig*  _config  = nullptr;
    StirrerControl* _control = nullptr;
    StirrerEncoder* _encoder = nullptr;
    StirrerDisplay* _display = nullptr;

    MenuPage      _page         = PAGE_STATUS;
    bool          _dirty        = true;   // redraw on the next update()
    unsigned long _lastRefresh  = 0;
    unsigned long _messageStart = 0;
    bool          _showMessage  = false;
    char          _shownLine1[StirrerDisplay::LCD_COLS + 1] = "";
    char          _shownLine2[StirrerDisplay::LCD_COLS + 1] = "";
};
