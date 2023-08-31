#ifndef FONTS_H
#define FONTS_H

#include <avr/io.h>
#include <avr/pgmspace.h>

#define MCFLETOFFSET ('A' - 10)
#define MCFFONTWIDTH 12
#define MCFFONTSPACER 2

#define MCFMINUS 36

#define MCFNULCHAR 0xFF  // this is considered a null character in the 'mnemonic' for an 'action' when less than 5 characters are needed


/* 1 byte sent to LCD at a time. Data is sent for 8 pixels, scanned vertically, and uses the Least Significant Bit (LSB) ordering (top pixel is the LSB).
 * 1 is a black pixel, 0 is a white pixel
 * In terms of rows, you usually would line up the pixels by 'page' (one page is 8 pixels high): page 0 is rows 0 to 7, page 1 is rows 8 to 15, etc (starting from the top)
 * In terms of columns, you just tell the LCD what column you want to draw to. One call to 'senddata' will draw 8 pixel high by 1 pixel wide.
 * In general, we send data column by column for a given page (form left to right), then once a page is fully drawn, we move to the next page.
 * 
 * Our font is '2 pages' high (16 rows). So we can draw text in pages 0 and 1 (on top, so 'Z' stack variable), skip page 2 (provide for enough blank space), draw numbers and 
 * text in pages 3 and 4 (middle, so 'Y' stack variable) leave page 5 blank, and draw in pages 6 and 7 (bottom, where we will show the 'X' stack variable). This will provide
 * for a gap of 8 pixels (a full page) between the X, Y and Z variables.
 * 
 * Alternatively in the future, we could consider a font that is 18 pixels high, and reduce the gap between 2 rows of numbers to 5 pixels. May or may not 'feel' too 'tight'.
 */

// 10 char, 12 col by 16 row (2 pages) each
static const uint8_t MiyoCalcFont[] PROGMEM = {
  0xf0,	0xfc,	0x0e,	0x07,	0x03,	0x03,	0x03,	0x07,	0x0e,	0xfc,	0xf0,	0x00,
  0x0f,	0x3f,	0x70,	0xe0,	0xc0,	0xc0,	0xc0,	0xe0,	0x70,	0x3f,	0x0f,	0x00,  // 0  @0
  
  0x00, 0x06, 0x06, 0x06, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xc0, 0xc0, 0xc0, 0xc0, 0xff, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0x00, 0x00,  // 1  @1
  
  0x04, 0x06, 0x02, 0x03, 0x03, 0x03, 0x03, 0xc6, 0xfe, 0x38, 0x00, 0x00, 
  0xc0, 0xe0, 0xf0, 0xf8, 0xdc, 0xce, 0xc7, 0xc1, 0xc0, 0xc0, 0xc0, 0x00,  // 2  @2
  
  0x00, 0x06, 0x06, 0x83, 0x83, 0x83, 0xc3, 0xc7, 0x7e, 0x3c, 0x00, 0x00,  
  0x20, 0x60, 0xe0, 0xc0, 0xc1, 0xc1, 0xc1, 0xc1, 0x67, 0x7e, 0x3c, 0x00,  // 3  @3
  
  0x00, 0x00, 0x80, 0xe0, 0x70, 0x38, 0x0e, 0x07, 0xff, 0xfe, 0x00, 0x00,  
  0x0e, 0x0f, 0x0f, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xff, 0x0c, 0x0c,  // 4  @4
  
  0x00, 0xfe, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x83, 0x83, 0x00, 0x00, 
  0x60, 0x60, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0x71, 0x3f, 0x1e, 0x00,  // 5  @5
  
  0x00, 0xf0, 0xfc, 0x8e, 0x86, 0x83, 0xc3, 0xc3, 0x83, 0x86, 0x06, 0x00,  
  0x01, 0x1f, 0x3f, 0x61, 0xc0, 0xc0, 0xc0, 0xc0, 0xe1, 0x7f, 0x3f, 0x04,  // 6  @6
  
  0x03, 0x03, 0x03, 0x03, 0x03, 0x83, 0xe3, 0x7b, 0x1f, 0x0f, 0x03, 0x00,  
  0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  // 7  @7
  
  0x00, 0x3c, 0x7e, 0xe7, 0xc3, 0x83, 0x83, 0x83, 0xe6, 0x7c, 0x18, 0x00,
  0x1c, 0x7e, 0x63, 0xc1, 0xc1, 0xc1, 0xc1, 0xc3, 0xe3, 0x7e, 0x3c, 0x00,  // 8  @8
  
  0x30, 0xfc, 0xfe, 0x07, 0x03, 0x03, 0x03, 0x03, 0x86, 0xfc, 0xf8, 0x00,  
  0x00, 0x60, 0xe1, 0xc3, 0xc3, 0xc3, 0xc3, 0xe1, 0x71, 0x3f, 0x0f, 0x00,  // 9  @9 
  
  0x00,	0x00,	0x00,	0xc0,	0xfc,	0x0f,	0x0f,	0xfc,	0xc0,	0x00,	0x00,	0x00,
  0x80,	0xf0,	0x7e,	0x07,	0x06,	0x06,	0x06,	0x06,	0x07,	0x7e,	0xf0,	0x80,  // A  @10
  
  0xff,	0xff,	0xc3,	0xc3,	0xc3,	0xc3,	0xc3,	0xe7,	0xbe,	0x1c,	0x00,	0x00,
  0xff,	0xff,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xe1,	0x71,	0x3f,	0x1e,	0x00,  // B  @11

  0x80, 0xf0, 0xfc, 0x0e, 0x06, 0x07, 0x03, 0x03, 0x03, 0x07, 0x06, 0x04,
  0x01, 0x0f, 0x3f, 0x70, 0x60, 0xe0, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x20,  // C  @12
  
  0xff, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x07, 0x0e, 0x3c, 0xf8, 0xe0,
  0xff, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0x70, 0x3c, 0x1f, 0x07,  // D  @13
  
  0xff, 0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x03, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00,  // E  @14
  
  0xff, 0xff, 0xff, 0x83, 0x83, 0x83, 0x83, 0x83, 0x83, 0x03, 0x00, 0x00, 
  0xff, 0xff, 0xff, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,  // F  @15
  
  0xe0, 0xf8, 0x1c, 0x0e, 0x07, 0x03, 0x83, 0x83, 0x83, 0x86, 0x86, 0x80,  
  0x07, 0x1f, 0x38, 0x70, 0xe0, 0xc0, 0xc1, 0xc1, 0xc1, 0xe1, 0x7f, 0x3f,  // G  @16
  
  0xff,	0xff,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xc0,	0xff,	0xff,
  0xff,	0xff,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0xff,	0xff,  // H  @17

  0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x00, 0x00,
  0xc0, 0xc0, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00,  // I  @18
  
  0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00,
  0x30, 0x60, 0xe0, 0xc0, 0xc0, 0xc0, 0xe0, 0x7f, 0x3f, 0x0f, 0x00, 0x00,  // J  @19
  
  0xff, 0xff, 0xff, 0x80, 0xc0, 0xe0, 0xf8, 0x1c, 0x0e, 0x07, 0x01, 0x00,
  0xff, 0xff, 0xff, 0x03, 0x01, 0x00, 0x03, 0x0f, 0x3e, 0x78, 0xe0, 0x80,  // K  @20
  
  0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00,  // L  @21
    
  0xff,	0xff,	0x0f,	0x3e,	0xf0,	0x80,	0x80,	0xf0,	0x3e,	0x0f,	0xff,	0xff,
  0xff,	0xff,	0x00,	0x00,	0x01,	0x0f,	0x0f,	0x01,	0x00,	0x00,	0xff,	0xff,  // M  @22
  
  0xff, 0xff, 0x8f, 0x3c, 0xf0, 0xc0, 0x80, 0x00, 0x00, 0xff, 0xff, 0x00,
  0xff, 0xff, 0xff, 0x00, 0x00, 0x03, 0x0f, 0x3e, 0xf0, 0xff, 0xff, 0x00,  // N  @23
  
  0xf0, 0xfc, 0x1e, 0x07, 0x03, 0x03, 0x03, 0x03, 0x06, 0x1e, 0xfc, 0xf0, 
  0x0f, 0x3f, 0x78, 0x60, 0xc0, 0xc0, 0xc0, 0xc0, 0x60, 0x78, 0x3f, 0x07,  // O  @24
  
  0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x87, 0xfe, 0xfc, 0x00,
  0x00, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00,  // P  @25
  
  0xf0,	0xfc,	0x1e,	0x07,	0x03,	0x03,	0x03,	0x03,	0x06,	0x1e,	0xfc,	0xf0,
  0x0f,	0x3f,	0x78,	0x60,	0xc0,	0xc0,	0xcc,	0xdc,	0x78,	0x70,	0xff,	0xc7,  // Q  @26

  0xff, 0xff, 0x83, 0x83, 0x83, 0x83, 0x83, 0xc7, 0xfe, 0x7e, 0x18, 0x00,
  0xff, 0xff, 0x01, 0x01, 0x01, 0x03, 0x0f, 0x3d, 0xf8, 0xe0, 0x80, 0x00,  // R  @27
  
  0x18, 0x7e, 0xfe, 0xc3, 0xc3, 0x83, 0x83, 0x83, 0x06, 0x06, 0x00, 0x00,
  0x30, 0x60, 0x60, 0xc0, 0xc0, 0xc1, 0xc1, 0xc3, 0x67, 0x7e, 0x1c, 0x00,  // S  @28
  
  0x03,	0x03,	0x03,	0x03,	0x03,	0xff,	0xff,	0x03,	0x03,	0x03,	0x03,	0x03,
  0x00,	0x00,	0x00,	0x00,	0x00,	0xff,	0xff,	0x00,	0x00,	0x00,	0x00,	0x00,  // T  @29
  
  0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00,
  0x00, 0x3f, 0x7f, 0xe0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0x7f, 0x3f, 0x00,  // U  @30
  
  0x07,	0x3f,	0xf0,	0x80,	0x00,	0x00,	0x00,	0x00,	0x80,	0xf0,	0x3f,	0x07,
  0x00,	0x00,	0x01,	0x0f,	0x78,	0xc0,	0xc0,	0x78,	0x0f,	0x01,	0x00,	0x00,  // V  @31

  0x1f,	0xff,	0xe0,	0x00,	0x80,	0xf0,	0xf0,	0x80,	0x00,	0xe0,	0xff,	0x1f,
  0x00,	0x1f,	0xff,	0xf0,	0x7f,	0x03,	0x03,	0x7f,	0xf0,	0xff,	0x1f,	0x00,  // W  @32

  0x03, 0x07, 0x1e, 0x78, 0xe0, 0xc0, 0xf0, 0x3c, 0x0f, 0x03, 0x01, 0x00,
  0xc0, 0xf0, 0x3c, 0x0e, 0x03, 0x03, 0x0f, 0x3c, 0x78, 0xe0, 0x80, 0x00,  // X  @33

  0x03, 0x0f, 0x3c, 0xf0, 0xc0, 0x00, 0x80, 0xe0, 0x78, 0x1e, 0x07, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00,  // Y  @34
  
  0x00, 0x03, 0x03, 0x03, 0x03, 0x83, 0xc3, 0x73, 0x3b, 0x0f, 0x07, 0x01, 
  0xc0, 0xe0, 0xf8, 0xdc, 0xcf, 0xc3, 0xc1, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,  // Z  @35
  
  0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,
  0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,  // -  @36
  
  0x00,	0x80,	0xc0,	0xe0,	0x60,	0x30,	0x38,	0x18,	0x0c,	0x06,	0x06,	0x00,
  0x00,	0x01,	0x03,	0x07,	0x06,	0x0c,	0x1c,	0x18,	0x30,	0x60,	0x60,	0x00,  // <  @37

  0x00,	0x06,	0x0e,	0x0c,	0x18,	0x30,	0x30,	0x60,	0xe0,	0xc0,	0x80,	0x00,
  0x00,	0x60,	0x70,	0x30,	0x18,	0x0c,	0x0c,	0x06,	0x07,	0x03,	0x01,	0x00,  // >  @38
  
  0x00,	0x80,	0x80,	0x80,	0x80,	0xf8,	0xf8,	0x80,	0x80,	0x80,	0x80,	0x00,
  0x00,	0x01,	0x01,	0x01,	0x01,	0x1f,	0x1f,	0x01,	0x01,	0x01,	0x01,	0x00,  // +  @39

  0x00,	0x00,	0xfe,	0xff,	0xc3,	0x63,	0x3f,	0x1e,	0x00,	0x00,	0x80,	0x80,
  0x3c,	0x7f,	0xe3,	0xc1,	0xc3,	0xce,	0xdc,	0x78,	0x30,	0x7c,	0xef,	0xc3,  // &  @40
  
  0x00,	0x00,	0x80,	0xc0,	0xc0,	0xc0,	0x80,	0x00,	0x00,	0x80,	0xc0,	0x00,
  0x00,	0x03,	0x01,	0x00,	0x00,	0x01,	0x03,	0x03,	0x03,	0x01,	0x00,	0x00,  // ~  @41

  0x00,	0x00,	0x00,	0x80,	0xe0,	0x38,	0x38,	0xe0,	0x80,	0x00,	0x00,	0x00,
  0x00,	0x10,	0x1c,	0x0f,	0x01,	0x00,	0x00,	0x01,	0x0f,	0x1c,	0x10,	0x00,  // ^  @42
  
  0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00,
  0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00,  // =  @43
  
  0x00, 0x02, 0x03, 0x01, 0x01, 0xc1, 0xc1, 0x73, 0x3f, 0x0e, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x60, 0xf3, 0xf3, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,  // ?  @44
  
  0x00,	0x00,	0x00,	0x00,	0x00,	0x80,	0xf0,	0x7c,	0x0f,	0x01,	0x00,	0x00,
  0x00,	0x00,	0x80,	0xf0,	0x3e,	0x0f,	0x01,	0x00,	0x00,	0x00,	0x00,	0x00,  // /  @45

  0x00,	0x00,	0x00,	0x00,	0x00,	0xff,	0xff,	0x00,	0x00,	0x00,	0x00,	0x00,
  0x00,	0x00,	0x00,	0x00,	0x00,	0xff,	0xff,	0x00,	0x00,	0x00,	0x00,	0x00,  // |  @46

  0x00,	0x00,	0x20,	0xe0,	0xe0,	0x20,	0x20,	0xe0,	0xe0,	0x20,	0x00,	0x00,
  0x00,	0x00,	0x00,	0x0f,	0x0f,	0x00,	0x00,	0x0f,	0x0f,	0x08,	0x00,	0x00,  // pi  @47

};

// 3 col x 16 rows
const PROGMEM uint8_t MiyoCalcFont_Dot [] = {
0x00, 0x00, 0x00, 
0xf0, 0xf0, 0xf0,    // .
};

// 6 col x 8 rows (one page)
const PROGMEM uint8_t MiyoCalcFont_Degree [] = {
0x3c,	0x7e,	0x42,	0x42,	0x7e,	0x3c,  // degree symbol
};

#endif
