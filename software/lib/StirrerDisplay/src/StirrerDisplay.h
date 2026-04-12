#ifndef StirrerDisplay_H // include guard
#define StirrerDisplay_H

#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h> 

class StirrerDisplay {

    public:
        StirrerDisplay();
        ~StirrerDisplay();
        void setup();
        void write(const char* test);
    private:
        hd44780_pinIO* lcd;
        //const int rs=12, en=13, db4=17, db5=16, db6=27, db7=14;
        const int rs=19, en=23, db4=18, db5=17, db6=26, db7=15;
        const int LCD_COLS = 16;
        const int LCD_ROWS = 2;

};

#endif //StirrerDisplay_H