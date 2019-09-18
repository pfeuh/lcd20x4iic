
#include "lcd20x4iic.h"

LCD_20X4_IIC lcd(0x27);

long milestone;
#define STEP_DURATION 300

const char splashText[] PROGMEM = "test  LCD I2C 20x4\nversion " LCD_20X4_IIC_VERSION "\n" __DATE__ " " __TIME__;
const char bigText[] PROGMEM = "Hello, world! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
const char line1[] PROGMEM = "line 1";
const char line2[] PROGMEM = "line 2";
const char line3[] PROGMEM = "line 3";
const char line4[] PROGMEM = "line 4";
const char displayModeMessage[] PROGMEM = " -* Display mode *- ";

//~ const int numbers[] PROGMEM = {1, 2, 3, 13, 123, 1234, 12345, -12345, -1234, -123, -13, -3, -2, -1, 0, -32768};
const int numbers[] PROGMEM = {-1, -12, -123, -1234, -12345, -23456, -30000, -30767, -30768};
#define NUMBER_OF_NUMBERS (sizeof(numbers) / sizeof(int))
    
const char withLF[] PROGMEM = "newline 1\nnewline 2\nnewline 3\nnewline 4\n";

const char newChars[] PROGMEM =
{
    B00000,B00000,B00000,B00000,B11111,B00000,B00000,B00000, // #00 line
    B00000,B00000,B00100,B01110,B11111,B01110,B00100,B00000, // #01 dotOnLine
    B00000,B00100,B01110,B11111,B00100,B00100,B00000,B00000, // #02 arrowUp 
    B00000,B00000,B00100,B00100,B11111,B01110,B00100,B00000, // #03 arrowDown
    B00000,B00000,B01110,B10001,B10001,B10001,B01110,B00000, // #04 radioOff
    B00000,B00000,B01110,B11111,B11011,B11111,B01110,B00000, // #05 radioOn
    B00000,B00000,B10001,B01010,B00100,B01010,B10001,B00000, // #06 checkOn
    B00000,B01110,B10001,B10001,B11111,B11011,B11111,B00000, // #07 lock
};

#define CHAR_LINE       0
#define CHAR_DOT        1
#define CHAR_ARROW_UP   2
#define CHAR_ARROW_DOWN 3
#define CHAR_RADIO_OFF  4
#define CHAR_RADIO_ON   5
#define CHAR_CHECK_ON   6
#define CHAR_LOCK       7

int x;

void testBacklight()
{
    for(x=0; x<3; x++)
    {
        lcd.setBacklight(0);
        delay(400);  
        lcd.setBacklight(1);
        delay(400);  
    }
}

void splashScreen()
{
    lcd.clear();
    lcd.home();
    lcd.println(splashText);
    delay(2000);
}

void testCursorAt()
{
    lcd.clear();
    lcd.cursorAt(0,3);
    lcd.print(line4);
    lcd.cursorAt(0,2);
    lcd.print(line3);
    lcd.cursorAt(0,1);
    lcd.print(line2);
    lcd.cursorAt(0,0);
    lcd.print(line1);
    delay(2000);  
}

void testRedefinedCharacters()
{
    lcd.write('<');
    lcd.write(CHAR_LINE);
    lcd.write(CHAR_DOT);
    lcd.write(CHAR_ARROW_UP);
    lcd.write(CHAR_ARROW_DOWN);
    lcd.write(CHAR_RADIO_OFF);
    lcd.write(CHAR_RADIO_ON);
    lcd.write(CHAR_CHECK_ON);
    lcd.write(CHAR_LOCK);
    lcd.write('>');
    delay(2000);  
}

void testLineFeed()
{
    lcd.clear();
    lcd.print(withLF);
    delay(2000);    
}

void testBigString()
{
    lcd.print(bigText);
    delay(2000);  
}

void testPrintNumber()
{
    lcd.clear();
    for(byte x=0; x<NUMBER_OF_NUMBERS;x++)
    {
        lcd.print(pgm_read_word(numbers + x));
        lcd.write(' ');
    }
    delay(2000);  
}

void testCarriageReturn()
{
    for(char x = -100; x < 101; x++)
    {
        lcd.write(LCD_CHAR_CR);
        lcd.write(' ');
        lcd.print(x);
        lcd.write(' ');
        delay(50);
    }
}

void testTabulation()
{
    lcd.clear();
    for(byte x = 0; x < 19; x++)
    {
        lcd.print(x);
        lcd.write(LCD_CHAR_TABULATION);
    }
    delay(2000);
    lcd.clear();
    lcd.setTabulation(10);
    for(byte x = 0; x < 9; x++)
    {
        lcd.print((char*)"fname");
        lcd.write(x);
        lcd.write(LCD_CHAR_TABULATION);
    }
    delay(2000);
}

void setup()
{
    Serial.begin(9600);

    lcd.begin();

    lcd.createChar(CHAR_LINE, newChars);
    lcd.createChar(CHAR_DOT, newChars + 8);
    lcd.createChar(CHAR_ARROW_UP, newChars + 16);
    lcd.createChar(CHAR_ARROW_DOWN, newChars + 24);
    lcd.createChar(CHAR_RADIO_OFF, newChars + 32);
    lcd.createChar(CHAR_RADIO_ON, newChars + 40);
    lcd.createChar(CHAR_CHECK_ON, newChars + 48);
    lcd.createChar(CHAR_LOCK, newChars + 56);
    
    testBacklight();
    splashScreen();
    //~ testCursorAt();
    //~ testRedefinedCharacters();
    //~ testLineFeed();
    //~ testBigString();
    //~ testPrintNumber();
    //~ testCarriageReturn();
    //~ testTabulation();
    
    lcd.setBlink(1);
    
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{   
    while(Serial.available())
        lcd.write(Serial.read());
    
    if(millis() & 0x200)
        digitalWrite(LED_BUILTIN, 0);
    else
        digitalWrite(LED_BUILTIN, 1);
}
