#include <StirrerDisplay.h>

StirrerDisplay::StirrerDisplay(){
    this->lcd = new LiquidCrystal(rs, en, db4, db5, db6, db7);
}

StirrerDisplay::~StirrerDisplay(){
    delete lcd;
}

void StirrerDisplay::setup(){
    pinMode(rs, OUTPUT);
    pinMode(en, OUTPUT);
    pinMode(db4, OUTPUT);
    pinMode(db5, OUTPUT);
    pinMode(db6, OUTPUT);
    pinMode(db7, OUTPUT);
    lcd->begin(LCD_COLS, LCD_ROWS);
}

void StirrerDisplay::write(uint8_t character){
    lcd->write(character);
}

void StirrerDisplay::print(const char* text){
    lcd->print(text);
}

void StirrerDisplay::clear(){
    lcd->clear();
}

void StirrerDisplay::home(){
    lcd->home();
}