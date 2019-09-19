/*
 * file : lcd20x4iic.cpp
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

#include "lcd20x4iic.h"
#include <Wire.h>

#define LCD_LAST_COLUMN (LCD_NB_COLUMNS - 1)
#define LCD_LAST_ROW (LCD_NB_ROWS - 1)
#define LCD_NB_CHARS (LCD_NB_COLUMNS * LCD_NB_ROWS)
#define LCD_LAST_LINE_OFFSET (LCD_LAST_ROW * LCD_NB_COLUMNS)

#define LCD_DEFAULT_TABULATION 4

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

#define LCD_20X4_IIC_NB_CUSTOM_CHARS 8
#define LCD_20X4_IIC_SECOND_LINE_OFFSET 0x40
#define LCD_20X4_IIC_CHARS_PER_ROW 0x14

const byte LCD_20X4_IIC_lineOffset[] PROGMEM =
{
    0x00,
    LCD_20X4_IIC_SECOND_LINE_OFFSET,
    LCD_20X4_IIC_CHARS_PER_ROW,
    LCD_20X4_IIC_SECOND_LINE_OFFSET + LCD_20X4_IIC_CHARS_PER_ROW
};

/*******************/
/* Private methods */
/*******************/

inline void LCD_20X4_IIC::command(byte value)
{
    send(value, 0);
}

void LCD_20X4_IIC::send(byte value, byte mode)
{
    byte highnib=value&0xf0;
    byte lownib=(value<<4)&0xf0;
    write4bits((highnib)|mode);
    write4bits((lownib)|mode); 
}

void LCD_20X4_IIC::write4bits(byte value)
{
    expanderWrite(value);
    pulseEnable(value);
}

void LCD_20X4_IIC::expanderWrite(byte _data)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write((int)(_data) | backlightValue);
    Wire.endTransmission();   
}

void LCD_20X4_IIC::pulseEnable(byte _data)
{
    expanderWrite(_data | En);
    delayMicroseconds(1);

    expanderWrite(_data & ~En);
    delayMicroseconds(50);
}

void LCD_20X4_IIC::gotoxy()
{
    splitPosition();
    command(LCD_SETDDRAMADDR | (x + pgm_read_byte(LCD_20X4_IIC_lineOffset + y)));
}

// these functions are console specific 

void LCD_20X4_IIC::splitPosition()
{
    x = position % LCD_NB_COLUMNS;
    y = position / LCD_NB_COLUMNS;
}

void LCD_20X4_IIC::joinPosition()
{
    position = x + y * LCD_NB_COLUMNS;
}

#ifdef LCD_SCROLL_UP
void LCD_20X4_IIC::scrollUp()
{
    char current_char;
    // scrolling up all rows except first
    x = 0;
    y = 0;
    joinPosition();
    for(byte cursor = LCD_NB_COLUMNS; cursor < LCD_NB_CHARS; cursor++)
    {
        current_char = screen[cursor];
        write(current_char);
        screen[cursor - LCD_NB_COLUMNS] = current_char;
    }
    
    // blanking last row
    x = 0;
    y = LCD_LAST_ROW;
    joinPosition();
    gotoxy();
    for(byte cursor = 0; cursor < LCD_NB_COLUMNS; cursor++)
    {
        i2c_write(' ');
        screen[cursor + LCD_LAST_LINE_OFFSET] = LCD_CHAR_SPACE;
    }
    gotoxy();
}
#endif

void LCD_20X4_IIC::incrementColumn()
{
    splitPosition();
    if(x < LCD_LAST_COLUMN)
        position++;
        // gotoxy automatically done by lcd...
        // if we stay on the current line
    else
        linefeed();
}

void LCD_20X4_IIC::backspace()
{
    if(position)
    {
        position--;
        gotoxy();
        write(' ');
        position--;
        splitPosition();
    }
    else
        i2c_write(' ');
    gotoxy();
}

void LCD_20X4_IIC::linefeed()
{
    splitPosition();
    if(y < LCD_LAST_ROW)
        position = (y + 1) * LCD_NB_COLUMNS;
    else
    {
        #ifdef LCD_SCROLL_UP
        scrollUp();
        position = LCD_LAST_ROW * LCD_NB_COLUMNS;
        #else
        position = 0;
        #endif
    }
    // as row has changed, we have to place the graphic cursor manually
    gotoxy();
}

void LCD_20X4_IIC::carriageReturn()
{
    splitPosition();
    x = 0;
    joinPosition();
    gotoxy();
}

void LCD_20X4_IIC::tabulation()
{
    splitPosition();
    byte old_x = x;
    byte new_x = ((x / tabulationValue) + 1 ) * tabulationValue;
    if(new_x > LCD_NB_COLUMNS)
        new_x = LCD_NB_COLUMNS;
    for(;old_x < new_x;old_x++)
        write(' ');
}

/******************/
/* Public methods */
/******************/

LCD_20X4_IIC::LCD_20X4_IIC(byte lcd_addr)
{
    i2cAddress = lcd_addr;
    backlightValue = LCD_NOBACKLIGHT;
    tabulationValue = LCD_DEFAULT_TABULATION;
}

void LCD_20X4_IIC::begin()
{
    Wire.begin();
    displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    reboot();  
}

void LCD_20X4_IIC::reboot()
{
    /***********************************************/
    /* Note, however, that resetting the Arduino   */
    /* doesn't reset the LCD, so we  can't assume  */
    /* that its in that state when a sketch starts */
    /***********************************************/

    displayFunction |= LCD_2LINE;

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
    delay(50); 

    // Now we pull both RS and R/W low to begin commands
    expanderWrite(backlightValue);	// reset expanderand turn backlight off (Bit 8 =1)
    delay(1000);

    //put the LCD into 4 bits mode
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

      // we start in 8bit mode, try to set 4 bits mode
    write4bits(0x03 << 4);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03 << 4);
    delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03 << 4); 
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02 << 4); 

    // set # lines, font size, etc.
    command(LCD_FUNCTIONSET | displayFunction);  

    // turn the display on with no cursor or blinking default
    displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    setDisplay(1);

    // clear it off
    clear();

    // Initialize to default text direction (for roman languages)
    displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    // set the entry mode
    command(LCD_ENTRYMODESET | displayMode);

    home();
}

void LCD_20X4_IIC::clear()
{
    command(LCD_CLEARDISPLAY);
    #ifdef LCD_SCROLL_UP
    for(byte cursor = 0; cursor < LCD_NB_CHARS; cursor++)
        screen[cursor] = ' ';
    #endif
    delayMicroseconds(2000);
    home();
}

void LCD_20X4_IIC::home()
{
    command(LCD_RETURNHOME);
    delayMicroseconds(2000);
    position = 0;
    gotoxy();
}

void LCD_20X4_IIC::setDisplay(byte on_off)
{
    if(on_off)
        displayControl |= LCD_DISPLAYON;
    else
        displayControl &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD_20X4_IIC::setBlink(byte on_off)
{
    if(on_off)
        displayControl |= LCD_BLINKON;
    else
        displayControl &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD_20X4_IIC::setCursor(byte on_off)
{
    if(on_off)
        displayControl |= LCD_CURSORON;
    else
        displayControl &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD_20X4_IIC::setBacklight(byte on_off)
{
    if(on_off)
        backlightValue = LCD_BACKLIGHT;
    else
        backlightValue = LCD_NOBACKLIGHT;
    expanderWrite(0);
}

inline size_t LCD_20X4_IIC::i2c_write(byte value)
{
    send(value, Rs);
    return 1;
}

void LCD_20X4_IIC::setTabulation(byte value)
{
    tabulationValue = value;
}
void LCD_20X4_IIC::cursorAt(byte _x, byte _y)
{
    x = _x;
    y = _y;
    joinPosition();
    gotoxy();
}

void LCD_20X4_IIC::print(char* strptr)
{
    while(*strptr)
        write(*strptr++);
}

void LCD_20X4_IIC::print(const char* strptr)
{
    while(pgm_read_byte(strptr))
        write(pgm_read_byte(strptr++));
}

void LCD_20X4_IIC::print(int number)
{
    // 5 digits are enough for a word
    byte digits[5];
    byte negative = 0;
    signed char cursor = 0;
    int result;
    int rest;
    
    if(number & 0x8000)
    {
        negative = 1;
        number = (~number) + 1;
    }
    
    if(!number)
        digits[cursor] = 0;
    else
    {
        while(1)
        {
            result = number / 10;
            rest = number % 10;
            if (rest == 0 && result == 0)
                break;
            digits[cursor] = rest;
            cursor++;
            number /= 10;
        }
        cursor -= 1;
    }
    
    if(negative)
        write('-');

    while(1)
    {
        if(cursor < 0)
            break;
        write(digits[cursor--] + '0');
    }
}

void LCD_20X4_IIC::println(char* strptr)
{
    print(strptr);
    write(LCD_CHAR_LF);
}

void LCD_20X4_IIC::println(const char* strptr)
{
    print(strptr);
    write(LCD_CHAR_LF);
}

void LCD_20X4_IIC::println(int number)
{
    print(number);
    write(LCD_CHAR_LF);
}

void LCD_20X4_IIC::createChar(byte char_num, const char* char_bytes_location)
{
    // there are only 8 locations 0...7
    char_num &= 0x7;
    command(LCD_SETCGRAMADDR | (char_num << 3));
    for (byte x=0; x<8; x++)
        i2c_write(pgm_read_byte(char_bytes_location + x));
}

void LCD_20X4_IIC::write(char car)
{
    if((car >= LCD_CHAR_SPACE) || (car < LCD_20X4_IIC_NB_CUSTOM_CHARS))
    {
        // only 0...7 & 0x20...0x7f displayed
        i2c_write(car);
        #ifdef LCD_SCROLL_UP
        screen[position] = car;
        #endif
        incrementColumn();
    }
    else
    {
        switch(car)
        {
            case LCD_CHAR_LF:
                linefeed();
                break;
            case LCD_CHAR_CR:
                carriageReturn();
                break;
            case LCD_CHAR_BACKSPACE:
                backspace();
                break;
            case LCD_CHAR_TABULATION:
                tabulation();
                break;
            //~ case LCD_CHAR_CLEAR:
                //~ clear();
                //~ gotoxy(0,0);
                //~ break;
            default:
                // unknown characters are rejected
                break;
        }
    }
}

