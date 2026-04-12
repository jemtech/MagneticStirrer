#include <StirrerDisplay.h>

StirrerDisplay::StirrerDisplay(){
    this->lcd = new hd44780_pinIO(rs, en, db4, db5, db6, db7);
}

StirrerDisplay::~StirrerDisplay(){
    delete lcd;
}

void StirrerDisplay::setup(){
    lcd->begin(LCD_COLS, LCD_ROWS);
}

void StirrerDisplay::write(const char* text){
    lcd->write(text);
}