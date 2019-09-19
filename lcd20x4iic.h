#ifndef lcd20x4iic_h
#define lcd20x4iic_h

/*
 * file : lcd20x4iic.h
 * Copyright (c) pfeuh <ze.pfeuh@gmail>
 * Heavyly inspired from
 * https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>

#define LCD_20X4_IIC_VERSION "1.02"

#define LCD_NB_COLUMNS 20
#define LCD_NB_ROWS 4
#define LCD_NB_CHARS (LCD_NB_COLUMNS * LCD_NB_ROWS)

#define LCD_CHAR_BACKSPACE  0x08
#define LCD_CHAR_TABULATION 0x09
#define LCD_CHAR_LF         0x0a
#define LCD_CHAR_CR         0x0d
#define LCD_CHAR_ESCAPE     0x1b
#define LCD_CHAR_SPACE      0x20

class LCD_20X4_IIC
{
    public:
        LCD_20X4_IIC(byte lcd_addr);
        void begin();
        void reboot();
        void clear();
        void home();
        void setDisplay(byte on_off);   
        void setBlink(byte on_off);
        void setCursor(byte on_off);
        void setBacklight(byte on_off);
        void createChar(byte, const char*);
        void command(byte);
        void print(char*);
        void print(const char*);
        void print(int);
        void println(char*);
        void println(const char*);
        void println(int);
        void write(char car);
        void cursorAt(byte x, byte y);
        void setTabulation(byte value);

    private:
        void gotoxy();
        void send(byte, byte);
        void write4bits(byte);
        void expanderWrite(byte);
        void pulseEnable(byte);
        virtual size_t i2c_write(byte);
        #ifdef LCD_SCROLL_UP
        void scrollUp();
        #endif
        void incrementColumn();
        void backspace();
        void linefeed();
        void carriageReturn();
        void tabulation();
        void splitPosition();
        void joinPosition();

        byte i2cAddress;
        byte displayFunction;
        byte displayControl;
        byte displayMode;
        byte backlightValue;
        byte tabulationValue;
        byte position;
        byte x;
        byte y;
        #ifdef LCD_SCROLL_UP
        char screen[LCD_NB_CHARS];
        #endif
};

#endif

