#pragma once
#include <string>

class StirrerDisplay {
public:
    static constexpr int LCD_COLS = 16;
    static constexpr int LCD_ROWS = 2;

    std::string lastLine1;
    std::string lastLine2;
    int         writeCount = 0;

    void setup() {}
    void write(const char* line1, const char* line2 = "") {
        lastLine1 = line1;
        lastLine2 = line2;
        writeCount++;
    }
    void clear() {
        lastLine1 = "";
        lastLine2 = "";
    }
};
