#include "StirrerDisplay.h"

StirrerDisplay::StirrerDisplay() {
    this->lcd = new hd44780_pinIO(rs, en, db4, db5, db6, db7);
}

StirrerDisplay::~StirrerDisplay() {
    delete lcd;
}

void StirrerDisplay::setup() {
    lcd->begin(LCD_COLS, LCD_ROWS);
    lcd->clear();
}

void StirrerDisplay::write(const char* line1, const char* line2) {
    writeLine(0, line1);
    writeLine(1, line2);
}

void StirrerDisplay::clear() {
    lcd->clear();
}

void StirrerDisplay::writeLine(uint8_t row, const char* text) {
    lcd->setCursor(0, row);
    int len = strlen(text);
    lcd->print(text);
    // Overwrite old characters with spaces
    for (int i = len; i < LCD_COLS; i++) {
        lcd->print(' ');
    }
}
