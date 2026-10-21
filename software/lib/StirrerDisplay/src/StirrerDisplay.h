#pragma once
#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h>

class StirrerDisplay {
public:
    StirrerDisplay();
    ~StirrerDisplay();
    void setup();
    void write(const char* line1, const char* line2 = "");
    void clear();

    // public so callers can size line buffers (LCD_COLS + 1)
    static constexpr int LCD_COLS = 16;
    static constexpr int LCD_ROWS = 2;

private:
    void writeLine(uint8_t row, const char* text);
    hd44780_pinIO* lcd;

    static constexpr int rs  = 19;
    static constexpr int en  = 23;
    static constexpr int db4 = 18;
    static constexpr int db5 = 17;
    static constexpr int db6 = 16;
    static constexpr int db7 = 15;
};
