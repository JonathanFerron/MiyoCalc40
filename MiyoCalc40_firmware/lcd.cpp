/* 
 * In part borrowed from the ERM19264_UC1609 project by Gavin Lyons: https://github.com/gavinlyonsrepo/ERM19264_UC1609
 * ERM19264 LCD driven by UC1609C controller
 * 
 * LCD draws about 270 to 290 uA at 3.2V (higher number is when more pixels are 'on')
 */

#include "lcd.h"

#include "fonts.h"

// Class Constructors
// Hardware SPI
ERM19264_UC1609_T  :: ERM19264_UC1609_T(int8_t cd, int8_t rst, int8_t cs) 
{
  _LCD_CD = cd;
  _LCD_RST= rst;
  _LCD_CS = cs;
}

// Desc: begin Method initialise LCD 
// Sets pinmodes and SPI setup
// Param1: VBiasPOT default = 0x49 , range 0x00 to 0xFE
void ERM19264_UC1609_T::LCDbegin(uint8_t VbiasPOT) 
{
  pinMode(_LCD_CD , OUTPUT);
  pinMode(_LCD_RST, OUTPUT);
  pinMode(_LCD_CS, OUTPUT);  
  _VbiasPOT  = VbiasPOT;
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_FREQ, SPI_DIRECTION, SPI_UC1609_MODE));  
  LCDinit();
}

// Desc: Called from LCDbegin carries out Power on sequence and register init
void ERM19264_UC1609_T::LCDinit()
{
  UC1609_CD_SetHigh;
  UC1609_CS_SetHigh;
  
  delay(UC1609_POWERON_DELAY1); 
  UC1609_RST_SetLow;
  delay(UC1609_POWERON_DELAY2); 
  UC1609_RST_SetHigh;
  delay(UC1609_POWERON_DELAY3);

  UC1609_CS_SetLow;

  send_command(UC1609_TEMP_COMP_REG, UC1609_TEMP_COMP_SET); 
  send_command(UC1609_ADDRESS_CONTROL, UC1609_ADDRESS_SET); 
  send_command(UC1609_FRAMERATE_REG, UC1609_FRAMERATE_SET);
  send_command(UC1609_BIAS_RATIO, UC1609_BIAS_RATIO_SET);  
  send_command(UC1609_POWER_CONTROL,  UC1609_PC_SET); 
  delay(UC1609_INIT_DELAY);
  
  send_command(UC1609_GN_PM, 0);
  send_command(UC1609_GN_PM, _VbiasPOT); //  changed by user
  
  send_command(UC1609_DISPLAY_ON, 0x01); // turn on display
  send_command(UC1609_LCD_CONTROL, UC1609_ROTATION_NORMAL); // rotate to normal 
  
  UC1609_CS_SetHigh;
}


// Desc: Sends a command to the display
// Param1: the command
// Param2: the values to change
void ERM19264_UC1609_T::send_command(uint8_t command, uint8_t value) 
{
  UC1609_CD_SetLow; 
  send_data(command | value);
  UC1609_CD_SetHigh;
}


// Desc: turns in display
// Param1: bits 1  on , 0 off
void ERM19264_UC1609_T::LCDEnable(uint8_t bits) 
{
 UC1609_CS_SetLow;
  send_command(UC1609_DISPLAY_ON, bits);
 UC1609_CS_SetHigh;
}


// Desc: Powerdown procedure for LCD see datasheet P40
void ERM19264_UC1609_T::LCDPowerDown(void)
{
  UC1609_RST_SetLow;
  delay(1);  // datasheet FIG 14 says >= 3uS
  UC1609_RST_SetHigh;
  LCDEnable(0);
}

// Desc: Fill the screen with a datapattern 
// Param1: datapattern can be set to zero to clear screen (not buffer) range 0x00 to 0ff
// Param2: optional delay in microseconds can be set to zero normally.
void ERM19264_UC1609_T::LCDFillScreen(uint8_t dataPattern=0, uint8_t delay=0) 
{
 UC1609_CS_SetLow;
  uint16_t numofbytes = LCD_WIDTH * (LCD_HEIGHT /8); // width * height
  for (uint16_t i = 0; i < numofbytes; i++) 
  {
    send_data(dataPattern);
    delayMicroseconds(delay);
  }
UC1609_CS_SetHigh;
}

//Desc: Draw a bitmap in PROGMEM to the screen
//Param1: x offset 0-192
//Param2: y offset 0-64
//Param3: width 0-192
//Param4 height 0-64
//Param5 the bitmap
void ERM19264_UC1609_T::LCDBitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t* data) 
{
  UC1609_CS_SetLow;

  uint8_t tx, ty; 
  uint16_t offset = 0; 
  uint8_t column = (x < 0) ? 0 : x;
  uint8_t page = (y < 0) ? 0 : y >>3;

  for (ty = 0; ty < h; ty = ty + 8) 
  {
    if (y + ty < 0 || y + ty >= LCD_HEIGHT) {continue;}
    send_command(UC1609_SET_COLADD_LSB, (column & 0x0F)); 
    send_command(UC1609_SET_COLADD_MSB, (column & 0xF0) >> 4);
    send_command(UC1609_SET_PAGEADD, page++); 

    for (tx = 0; tx < w; tx++) 
    {
      if (x + tx < 0 || x + tx >= LCD_WIDTH) {continue;}
      offset = (w * (ty >> 3)) + tx; 
      send_data(pgm_read_byte(&data[offset]));
    }
  }
  UC1609_CS_SetHigh;
}

//Desc: Send data byte with SPI to UC1609C
//Param1: the data byte
void ERM19264_UC1609_T::send_data(uint8_t byte)
{
  (void)SPI.transfer(byte); // Hardware SPI
} 

// Desc: goes to XY position
// Param1 : column 0-192
// Param2  : page 0-7
void ERM19264_UC1609_T::LCDGotoXY(uint8_t column , uint8_t page)
{
  UC1609_CS_SetLow;
  send_command(UC1609_SET_COLADD_LSB, (column & 0x0F)); 
  send_command(UC1609_SET_COLADD_MSB, (column & 0xF0) >> 4);
  send_command(UC1609_SET_PAGEADD, page++); 
  UC1609_CS_SetHigh;
}

// param : index, provide index of character from MiyoCalcFont table
// Param : column 0-192
// Param  : page 0-7
void ERM19264_UC1609_T::LCDChar(uint8_t index, uint8_t col, uint8_t page)
{
  UC1609_CS_SetLow;
  send_command(UC1609_SET_COLADD_LSB, (col & 0x0F)); 
  send_command(UC1609_SET_COLADD_MSB, (col & 0xF0) >> 4);
  send_command(UC1609_SET_PAGEADD, page++); 
  for (uint8_t column = 0 ; column <  MCFFONTWIDTH ; column++)
  {
    send_data(pgm_read_byte(MiyoCalcFont + index * MCFFONTWIDTH * 2 + column));
  }
  
  send_command(UC1609_SET_COLADD_LSB, (col & 0x0F)); 
  send_command(UC1609_SET_COLADD_MSB, (col & 0xF0) >> 4);
  send_command(UC1609_SET_PAGEADD, page++); 
  for (uint8_t column = 0 ; column <  MCFFONTWIDTH ; column++)
  {
    send_data(pgm_read_byte(MiyoCalcFont + index * MCFFONTWIDTH * 2 + MCFFONTWIDTH + column));
  }
  UC1609_CS_SetHigh;
}

// Param1 : column 0-192
// Param2  : page 0-7
void ERM19264_UC1609_T::LCDDot(uint8_t col, uint8_t page)
{
  UC1609_CS_SetLow;
  send_command(UC1609_SET_COLADD_LSB, (col & 0x0F)); 
  send_command(UC1609_SET_COLADD_MSB, (col & 0xF0) >> 4);
  send_command(UC1609_SET_PAGEADD, page++); 
  for (uint8_t column = 0 ; column <  3 ; column++)
  {
    send_data(pgm_read_byte(MiyoCalcFont_Dot + column));
  }
  
  send_command(UC1609_SET_COLADD_LSB, (col & 0x0F)); 
  send_command(UC1609_SET_COLADD_MSB, (col & 0xF0) >> 4);
  send_command(UC1609_SET_PAGEADD, page++); 
  for (uint8_t column = 0 ; column <  3 ; column++)
  {
    send_data(pgm_read_byte(MiyoCalcFont_Dot + 3 + column));
  }
  UC1609_CS_SetHigh;
}


// Desc: draws passed  character array
// Param1: pointer to start of character array
// careful with this: the function should only be called with upper case letters. No symbols, no numbers, no lower case.
// Param1 : column 0-192
// Param2  : page 0-7
void ERM19264_UC1609_T::LCDString(char *characters, uint8_t col, uint8_t page)
{
  uint8_t i = 0;
  while (*characters)
  {
    LCDChar(*characters++ - MCFLETOFFSET, col + i * (MCFFONTWIDTH + MCFFONTSPACER), page);
    i++;
  }
}

void ERM19264_UC1609_T::LCDCharSeq(uint8_t indexes[], uint8_t size, uint8_t col, uint8_t page)
{
  for (uint8_t i=0; i < size; i++)
  {
    LCDChar(indexes[i], col + i * (MCFFONTWIDTH + MCFFONTSPACER), page);
  }
}

// set contrast, values from 0x0A to 0x20 tend to work well
void ERM19264_UC1609_T::LCDSetContrast(uint8_t cont)
{
  UC1609_CD_SetLow;
  UC1609_CS_SetLow;
  (void)SPI.transfer(UC1609_GN_PM | 0);
  UC1609_CS_SetHigh;

  UC1609_CD_SetLow;
  UC1609_CS_SetLow;
  (void)SPI.transfer(UC1609_GN_PM | cont);
  UC1609_CS_SetHigh;
}
