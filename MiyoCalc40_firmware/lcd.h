/*
   Based in part on code from the ERM19264_UC1609 project by Gavin Lyons: https://github.com/gavinlyonsrepo/ERM19264_UC1609
*/

#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "gpio.h"

// Display Pixel colours   definition
// black on white, FG = black BG = white
// ERM19264FS-4 LCD Display Black on White
#define FOREGROUND  0
#define BACKGROUND 1
#define INVERSE 2

// UC1909 Read registers
#define UC1609_GET_STATUS 0xFE

// UC1909 Write registers
#define UC1609_SYSTEM_RESET 0xE2

#define UC1609_POWER_CONTROL 0x28
#define UC1609_PC_SET 0x06 // PC[2:0] 110 Internal V LCD (7x charge pump) + 10b: 1.4mA

#define UC1609_ADDRESS_CONTROL 0x88 // set RAM address control
#define UC1609_ADDRESS_SET 0x02 // Set AC [2:0] Program registers  for RAM address control.

#define UC1609_SET_PAGEADD 0xB0 // Page address Set PA[3:0]
#define UC1609_SET_COLADD_LSB 0x00 // Column Address Set CA [3:0]
#define UC1609_SET_COLADD_MSB 0x10 // Column Address Set CA [7:4]

#define UC1609_TEMP_COMP_REG 0x27 // Temperature Compensation Register
#define UC1609_TEMP_COMP_SET 0x00 // TC[1:0] = 00b= -0.00%/ C

#define UC1609_FRAMERATE_REG 0xA0 // Frame rate
#define UC1609_FRAMERATE_SET 0x01  // Set Frame Rate LC [4:3] 01b: 95 fps

#define UC1609_BIAS_RATIO 0xE8 // Bias Ratio. The ratio between V-LCD and V-D .
#define UC1609_BIAS_RATIO_SET 0x03 //  Set BR[1:0] = 11 (set to 9 default)

#define UC1609_GN_PM 0x81 // Set V BIAS Potentiometer to fine tune V-D and V-LCD  (double-byte command)
#define UC1609_DEFAULT_GN_PM 0x49 // default only used if user does not specify Vbias

#define UC1609_LCD_CONTROL 0xC0 // Rotate map control
#define UC1609_DISPLAY_ON 0xAE // enables display
#define UC1609_ALL_PIXEL_ON 0xA4 // sets on all Pixels on
#define UC1609_INVERSE_DISPLAY 0xA6 // inverts display
#define UC1609_SCROLL 0x40 // scrolls , Set the scroll line number. 0-64

// Rotate
#define UC1609_ROTATION_FLIP_TWO 0x06
#define UC1609_ROTATION_NORMAL 0x04
#define UC1609_ROTATION_FLIP_ONE 0x02
#define UC1609_ROTATION_FLIP_THREE 0x00

// Delays
// Delays
// mS , datasheet FIG 11 wait <=  3mS
#define UC1609_POWERON_DELAY1  3
// mS, datasheet FIG 11 wait >=  3uS
#define UC1609_POWERON_DELAY2  50
// mS  datasheet FIG 11 wait >= 5mS  , Does not work on SW SPI blue
#define UC1609_POWERON_DELAY3  0

#define UC1609_INIT_DELAY 100 //  mS delay ,after init, 


// No font
#ifdef UC_FONT_MOD_ONE
  #define UC1609_ASCII_OFFSET 0x00
#else
  #define UC1609_ASCII_OFFSET 0x20 //0x20, ASCII character for Space
#endif

#define UC1609_FONTPADDING  send_data(0x00)
#define UC1609_FONTWIDTH 5

// GPIO
#define UC1609_CS_SetHigh GPIO_HIGH(_LCD_CS)
#define UC1609_CS_SetLow GPIO_LOW(_LCD_CS)
#define UC1609_CD_SetHigh GPIO_HIGH(_LCD_CD)
#define UC1609_CD_SetLow GPIO_LOW(_LCD_CD)
#define UC1609_RST_SetHigh GPIO_HIGH(_LCD_RST)
#define UC1609_RST_SetLow GPIO_LOW(_LCD_RST)

// SPI: frequency/mode are fixed in avrducore/spi.c (6MHz, MSB-first, mode 0)

// Display  Size
const uint8_t LCD_WIDTH = 192;
const uint8_t LCD_HEIGHT = 64;

//class
class ERM19264_UC1609_T
{
public:
  // Contructor 1 Software SPI with explicit SCLK and SDIN
  ERM19264_UC1609_T(int8_t cd, int8_t rst, int8_t cs, int8_t sclk, int8_t din);
  // Constructor 2 Hardware SPI
  ERM19264_UC1609_T(gpio_pin_t cd, gpio_pin_t rst, gpio_pin_t cs);

  ~ERM19264_UC1609_T() {};

  void LCDGotoXY(uint8_t column, uint8_t page);
  void LCDString(const char* characters, uint8_t col, uint8_t page);
  void LCDbegin(uint8_t VbiasPot = UC1609_DEFAULT_GN_PM);
  void LCDinit(void);
  void LCDPowerDown(void);
  void FullLCDPowerDown(void);
  void LCDEnable(uint8_t on);
  void LCDFillScreen(uint8_t pixel, uint8_t mircodelay);
  void LCDFillPage(uint8_t pixels);
  void LCDBitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t* data);
  void LCDSetContrast(uint8_t cont);
  void LCDChar(uint8_t index, uint8_t col, uint8_t page);
  void LCDDot(uint8_t col, uint8_t page);
  void LCDCharSeq(uint8_t indexes[], uint8_t size, uint8_t col, uint8_t page);

private:

  void send_data(uint8_t data);
  void send_command(uint8_t command, uint8_t value);

  gpio_pin_t _LCD_CS;
  gpio_pin_t _LCD_CD;
  gpio_pin_t _LCD_RST;
   uint8_t _VbiasPOT; // Contrast default 0x49 datasheet 00-FE
};

#endif
