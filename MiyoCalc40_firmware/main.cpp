 /* Other notes:
  
  calc, config and programming mode: should be stored in a global status field that can be set and checked, could be an enum
  f, g, h shift modifiers: should each be a bool global variable that can be set and checked
  current config screen: should be stored in a global status fiels as well that can be set and checked, could be an enum (only relevant in 'config' mode)
  
  see repocalc, openrpncalc, dcalc (main.c) for examples
        H:\My Drive\MiyoCalc40\resources\openrpncalc
        H:\My Drive\MiyoCalc40\resources\repocalc
        H:\My Drive\MiyoCalc40\resources\dcalc-2.12
       
  MCU draws about 6.0mA at 3.2V when actively running at 24 MHz
     
*/

#include <stdbool.h>
#include <avr/sleep.h>

#include "matrix.h"
#include "lcd.h"
#include "fonts.h"
#include "main.h"
#include "cards.h"
#include "calc.h"

#define LCD_Default_Contrast 0x0C // Default LCD constrast: 0x0A to 0x20 tend to work well. Consider renaming this to something like LCD_DEF_Contrast.

// GPIO 5-wire SPI interface

/* //uncomment for arduino uno
#define CS 10  // GPIO pin number pick any you want
#define CD  9 // GPIO pin number pick any you want 
#define RST 8 // GPIO pin number pick any you want
// GPIO pin number SDA(UNO 11) , HW SPI , MOSI
// GPIO pin number SCK(UNO 13) , HW SPI , SCK
*/

// uncomment for avr-da
#define LCD_CS PIN_PA7  // AVR pin connected to LCD CS pin
#define LCD_CD PIN_PA2 // AVR pin connected to LCD CD pin
#define LCD_RST PIN_PA3 // AVR pin connected to LCD Reset pin


// Global variables
ERM19264_UC1609_T  mylcd(LCD_CD, LCD_RST, LCD_CS); // construct object using hardware SPI: CD, RST, CS

int lcdon;  // to track if lcd is turned on or not

int main() {
//  onBeforeInit(); // Empty callback called before init but after the .init stuff. First normal code executed
  init(); // implemented in wiring.c
  // by default, dxcore enables TCA0 timers at startup to use with pwm. see megaavr/extras/refcallbacks.md
//  initVariant();

  /* Insert here any code that needs to run before interrupts are
   * enabled but after all other core initialization. */
  sei();  // enable interrupts (could consider turning on interupts only after the setup() function, if it matters at all).
  
  setup();
  
  for (;;) {
    loop();
  }
}

void setup() {
   
  setupMCU();
  setupMatrix();
  
  calc_init();  
   
  setupLCD();
    
  setupBacklight();  
 
} // setup()

void setupMCU()
{
  // look into turning off TCD0 to save power (call takeoverTCD0() perhaps )
  
  // setup ADC for battery voltage: consider moving this to a separate setupBattVoltMonitor() function
  VREF.ADC0REF = VREF_REFSEL_2V048_gc;     // Set the ADC's voltage reference to 2.048V.
  VREF.ACREF = VREF_REFSEL_VDD_gc;         // Set the Analog Comparator's shared voltage reference to VDD.
  AC0.DACREF = 128;                        // set DACREF variable to 128 (will be divider by 256)
  ADC0.MUXPOS = ADC_MUXPOS_DACREF0_gc;     // Measure DACREF0
  ADC0.CTRLC = ADC_PRESC_DIV64_gc;         // 375kHz clock (24,000,000 / 64)
  ADC0.CTRLA = ADC_ENABLE_bm;              // Single, 12-bit, should move this inside the voltage calculation function, turn on before measuring, measure, then turn off when done
  //ADC0.CTRLA |= ADC_ENABLE_bm;
  
  set_sleep_mode(SLEEP_MODE_STANDBY);  
  sleep_enable();
}

void setupLCD()
{
  mylcd.LCDbegin(LCD_Default_Contrast); // initialize the LCD
  mylcd.LCDFillScreen(0x00, 0); // clear screen  
  delay(50);
  lcdon = true;
}

// pwm set-up on pin A1
void setupBacklight()  // this should be moved to the backlight file
{
  pinMode(PIN_PA1, OUTPUT); // set LCD LED Backlight pin to output mode
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc;  // route TCA to port A for PWM. Pin PA1 is on WO1 (waveform output 1)
  // see ref_timers.md in dxcore documentation
  // tca0 is configured by default as 8 bit timer with 6 output channels (we'll only need one): this allows for 256 different values for the duty cycle
  // we'll use the default prescaler which should give a frequency of 1471 Hz (this is with a prescale of 64). Could be changed to a prescaler of 256, but then 
  // frequency may be too low at 368 Hz? This would be done via TCA0.SPLIT.CTRLA = (TCA0.SPLIT.CTRLA & ~(0b00001110)) | TCA_SPLIT_CLKSEL_DIV256_gc
}


/*
go in sleep mode (enable sleep, enable bothedges interrups for all column pins) : see dxcore/megaavr/extras/powersave.md
		
upon wake interupt (port isr wake-up button press wakes up the mcu from sleep)					
  debounce 3ms to 20ms (10 times 2ms) using PIT (periodic interupt timer, programmed to trigger an interrupt every millisecond, lowpower_delay_ms function, set interrupt 
  to fire every 32 RTC clock cycles and enable PIT, enable PIT interrupts), validating logic low level on a column each time: btn_debounce() function					
  
  if logic low level still detected in a column					
    then scan keys (scan matrix) via function call: scankb()			
    
    if calc is off, power it up if 'on' button was pressed      
    else (lcd is on)
      if in mem store, recall mode or clear mode (see the mem_recall_mode, mem_store_mode and mem_clear_mode boolean variables):
        call mem store, recall, or clear with key pos (2D), should have a max of 40 possible var : call apply_memory_rcl(uint8_t r, uint8_t c), apply_memory_sto(uint8_t r, uint8_t c) and apply_memory_clr(uint8_t r, uint8_t c)
      else
        with key position (from scankb), lookup function pointer and keycode (action struct)
        
        if in 'calc mode'
          if 'off' button (combination) was pressed (can tell based on action), power down            
          else
            process action via function call (from function pointer): from 'action' struct from 'calc' (if in calc mode) or 'config' (if in config mode)
              provide it the keycode that was also looked up
              action in program mode, when a key is pressed to be recorded, is a function that will call another function with a keycode to log/record it					
          end if
        end if
      end if
    endif
    
    inner loop begin (idle until key is released):					
      if logic low level on any column still					
        then sleep					
          port isr wake-up: button is released					
      end if					
    end inner loop					
    may want to debouce here as well (openrpncalc uses 10ms)				
  end if					
 */
void loopNew() 
{  
  // scan keys
  scanKB();
  
  // if lcd is off, power it up if 'on' button was pressed
  if (keypos_r == ONOFFKEYPOS_R && keypos_c == ONOFFKEYPOS_C && !lcdon)
  {
    setupLCD();
    lcdon = true;
    set_sleep_mode(SLEEP_MODE_STANDBY);
  } // 'power on' lcd
 
  // if non-null keypos:
  if (keypos_r != 0xff)
  {
    
    // lookup function pointer+keycode (action struct)
    action current_action = keytoaction();
    keypos_c = 0xff; keypos_r = 0xff; // reset keypos to 'null' after action is obtained
    
    // process action via function call: from 'action' struct, call the function and provide it the keycode that was also looked up. 
    current_action.fct(current_action.keycode);
    
    // refresh lcd : this will become obsolete since we'll refresh the lcd from within the actions, when necessary
    mylcd.LCDFillScreen(0x00, 0); // clear screen
    mylcd.LCDChar('R' - MCFLETOFFSET, 14*0, 0*3);  // R
    mylcd.LCDChar(keypos_r, 14*1, 0 * 3);
    mylcd.LCDChar('C' - MCFLETOFFSET, 14*3, 0*3); // C 
    mylcd.LCDChar(keypos_c, 14*4, 0 * 3);  
    
    
  } // end processing of non-full keypos 
  
} // loop()

// old loop() code for testing
void loop() {
  //testContrast();
  //testStillNumbers();
  //testStillNumbers2();  // rename to testStack()
  //testActionsWithoutKeyboard();
  simulateInput();
  //testSomeText();
  //testSomeOtherText();
  //testBatteryVoltage();
  
  // test backlight
  //digitalWrite(PIN_PA1, HIGH);
  //delay(10000);
  //digitalWrite(PIN_PA1, LOW);
//  testBitmap();
} // loopOld()


// see http://www.technoblogy.com/list?3KLH
// http://www.technoblogy.com/show?3K82
// http://www.technoblogy.com/show?3KJR
/** 
 * Measuring VDD on an AVR128DA28

The ADC.MUXPOS register on the AVR128DA28 lets you choose to read a voltage reference option: DACREF0

It turns out that this is generated by the AC (Analog Comparator) peripheral. DACREF0 is:
AC0.DACREF / 256 × VREF.ACREF

where VREF.ACREF is the single Analog Comparator voltage reference, and AC0.DACREF 
is a byte stored in the DACREF register for Analog Comparator 0 which sets a voltage divider.

The AC0.DACREF value defaults to 255, so DACREF0 is initially 255/256 times the Analog Comparator VREF setting. Note that the 
255/256 factor is significant. It makes the default voltage references from DACREF0 1.02V rather than the 1.024V you might expect.

Setting up the ADC:

The procedure to set up the ADC is:

    Set the ADC's voltage reference to 1.024V.
    Set the Analog Comparator's shared voltage reference to VDD.
    Set Analog Comparator 0's DACREF value to 32.
    Set the ADC MUXPOS so the ADC measures DACREF0.

Here's the code to implement this:

void ADCSetup () {
  VREF.ADC0REF = VREF_REFSEL_1V024_gc;
  VREF.ACREF = VREF_REFSEL_VDD_gc;
  AC0.DACREF = 32;                                     // Maximum DACREF0 voltage
  ADC0.MUXPOS = ADC_MUXPOS_DACREF0_gc;                 // Measure DACREF0
  ADC0.CTRLC = ADC_PRESC_DIV64_gc;                     // 375kHz clock
  ADC0.CTRLA = ADC_ENABLE_bm;                          // Single, 12-bit
}

The datasheet specifies that the ADC clock should be at least 150kHz, so I've chosen a divider of 64 which gives a clock of 375kHz with a 24MHz processor clock.

Reading the voltage

Here's the routine to measure the supply voltage:

void MeasureVoltage () {
  ADC0.COMMAND = ADC_STCONV_bm;                        // Start conversion
  while (ADC0.COMMAND & ADC_STCONV_bm);                // Wait for completion
  uint16_t adc_reading = ADC0.RES;                     // ADC conversion result
  uint16_t voltage = adc_reading/50;
  Buffer[0] = voltage/10; Buffer[1]= voltage%10;
}

The ADC is now measuring 32/256 or 1/8 of VDD, which will always be within range of the 1.024 voltage reference.

The calculation is:
Result / 4096 × 1.024 = V / 8

So:
V = Result / 500, in volts

To get VDD in tenths of a volt we therefore need to divide the ADC reading by 50.

The maximum voltage we can measure this way is 4095/500 or 8.19V, which is well above the maximum supply voltage of 5.5V.

 * 
 **/
void testBatteryVoltage()  // move this to the util.c file
{
  ADC0.COMMAND = ADC_STCONV_bm;                        // Start conversion
  while (ADC0.COMMAND & ADC_STCONV_bm);                // Wait for completion
  uint16_t adc_reading = ADC0.RES;                     // ADC conversion result
  //uint16_t voltage = adc_reading/5;                    // convert to voltage * 100
  uint16_t voltage = adc_reading;
  //int Buffer[3] = {0, 0, 0};
  int Buffer[4] = {0, 0, 0, 0};
  Buffer[0] = voltage/1000; Buffer[1]= (voltage%1000)/100; Buffer[2] = (voltage%100)/10; Buffer[3] = voltage % 10;  
  
  // print out battery voltage on lcd
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(Buffer[0], 0, 0);
  mylcd.LCDChar(Buffer[1], MCFFONTWIDTH+MCFFONTSPACER, 0);
  mylcd.LCDChar(Buffer[2], (MCFFONTWIDTH+MCFFONTSPACER) * 2, 0);
  mylcd.LCDChar(Buffer[3], (MCFFONTWIDTH+MCFFONTSPACER) * 3, 0);
  delay(2000);
}

void loopTestBacklightPWM()
{
  for (int i = 0; i <= 10; i++)
  {
    int dutycycle = min(26*i, 254);
    analogWrite(PIN_PA1, dutycycle);
    mylcd.LCDFillScreen(0x00, 0); // clear screen
    mylcd.LCDChar(dutycycle / 100, 0, 0);
    mylcd.LCDChar((dutycycle % 100)/ 10, MCFFONTWIDTH+MCFFONTSPACER, 0);
    mylcd.LCDChar(dutycycle % 10, (MCFFONTWIDTH+MCFFONTSPACER) * 2, 0);
    delay(5000);
  }  
}

// shows how we would print numbers when displaying 3 elements of the stack (X at bottom, Y in middle, and Z on top)
// use this as a template for the DrawNum() function in calc.cpp (which will print to the LCD on a given page from a given number_for_lcd struct)
void testStillNumbers()
{
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  /*
  mylcd.LCDChar(3, 14*0, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDDot(14*1, 0*3); // col 14-16
  mylcd.LCDChar(1, 14*1 + 5, 0 * 3); // col 19-30
  mylcd.LCDChar(4, 14*2 + 5, 0 * 3); // col 33-44
  mylcd.LCDChar(1, 14*3 + 5, 0 * 3); 
  mylcd.LCDChar(5, 14*4 + 5 + 3, 0 * 3); 
  mylcd.LCDChar(9, 14*5 + 5 + 3, 0 * 3); 
  mylcd.LCDChar(2, 14*6 + 5 + 3, 0 * 3); 
  mylcd.LCDChar(6, 14*7 + 5 + 3*2, 0 * 3);
  mylcd.LCDChar(5, 14*8 + 5 + 3*2, 0 * 3);   
  */
  
  /*
  mylcd.LCDChar(3, 14*0, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDDot(14*1, 0*3); // col 14-16
  mylcd.LCDChar(1, 14*1 + 5, 0 * 3); // col 19-30
  mylcd.LCDChar(4, 14*2 + 5, 0 * 3); // col 33-44
  mylcd.LCDChar(1, 14*3 + 5, 0 * 3); 
  mylcd.LCDChar(5, 14*4 + 5 + 4, 0 * 3); 
  mylcd.LCDChar(9, 14*5 + 5 + 4, 0 * 3); 
  mylcd.LCDChar(2, 14*6 + 5 + 4, 0 * 3); 
  mylcd.LCDChar(6, 14*7 + 5 + 4*2, 0 * 3);
  mylcd.LCDChar(5, 14*8 + 5 + 4*2, 0 * 3);
  */
  
  number_for_lcd numforlcdZ;
  
  numforlcdZ.digits[0] = 3; numforlcdZ.digits[1] = 1; numforlcdZ.digits[2] = 4;
  numforlcdZ.digits[3] = 1; numforlcdZ.digits[4] = 5; numforlcdZ.digits[5] = 9;
  numforlcdZ.digits[6] = 2; numforlcdZ.digits[7] = 6; numforlcdZ.digits[8] = 5;  
  numforlcdZ.sign = 0;  // positive
  numforlcdZ.dec_point_pos = 1;
  numforlcdZ.num_digits = 9;
  numforlcdZ.show_dec_point = true;
  
  uint8_t col = 0;
  
  for (uint8_t d = 0; d < numforlcdZ.num_digits; d++)
  {
    if (d == numforlcdZ.dec_point_pos && numforlcdZ.show_dec_point)
    {
      mylcd.LCDDot(col, ZLCDPAGE);
      col += MCFDECPOINTWIDTH + MCFFONTSPACER;
    }
    
    mylcd.LCDChar(numforlcdZ.digits[d], col, ZLCDPAGE);
    col += MCFFONTWIDTH + MCFFONTSPACER;
    
    int8_t thousandcheck = d + 1 - numforlcdZ.dec_point_pos;
    
    if ( (thousandcheck % 3 == 0) && (thousandcheck != 0) )
    {
      col += MCFTHOUSANDSPACER;
    }     
  }
  
  if (numforlcdZ.dec_point_pos == numforlcdZ.num_digits && numforlcdZ.show_dec_point)
  {
    mylcd.LCDDot(col, ZLCDPAGE);
  }
  
  /*
  mylcd.LCDChar(5, 14*0, 1 * 3); // col 0, page 3-4
  mylcd.LCDChar(6, 14*1, 1 * 3); // 
  mylcd.LCDChar(0, 14*2+3, 1 * 3); // 
  mylcd.LCDChar(0, 14*3+3, 1 * 3); // 
  mylcd.LCDChar(0, 14*4+3, 1 * 3); // 
  mylcd.LCDChar(0, 14*5+3*2, 1 * 3); // 
  mylcd.LCDChar(0, 14*6+3*2, 1 * 3); // 
  mylcd.LCDChar(0, 14*7+3*2, 1 * 3); // 
  
  mylcd.LCDChar(2, 14*0, 2 * 3); // col 0, page 6-7
  mylcd.LCDDot(14*1, 2*3); // col 14-16
  mylcd.LCDChar(7, 14*1 + 5, 2 * 3); // col 19-30
  mylcd.LCDChar(1, 14*2 + 5, 2 * 3); // col 33-44
  mylcd.LCDChar(8, 14*3 + 5, 2 * 3); 
  mylcd.LCDChar(2, 14*4 + 5 + 3, 2 * 3); 
  mylcd.LCDChar(8, 14*5 + 5 + 3, 2 * 3); 
  mylcd.LCDChar(1, 14*6 + 5 + 3, 2 * 3); 
  mylcd.LCDChar(8, 14*7 + 5 + 3*2, 2 * 3);
  mylcd.LCDChar(3, 14*8 + 5 + 3*2, 2 * 3);   
  */
  
  
  number_for_lcd numforlcdY;
  
  numforlcdY.digits[0] = 5; numforlcdY.digits[1] = 6; numforlcdY.digits[2] = 0;
  numforlcdY.digits[3] = 0; numforlcdY.digits[4] = 0; numforlcdY.digits[5] = 0;
  numforlcdY.digits[6] = 0; numforlcdY.digits[7] = 0;  
  numforlcdY.sign = 0;  // positive
  numforlcdY.dec_point_pos = 8;
  numforlcdY.num_digits = 8;
  numforlcdY.show_dec_point = false;
  
  col = 0;
  
  for (uint8_t d = 0; d < numforlcdY.num_digits; d++)
  {
    if (d == numforlcdY.dec_point_pos && numforlcdY.show_dec_point)
    {
      mylcd.LCDDot(col, YLCDPAGE);
      col += MCFDECPOINTWIDTH + MCFFONTSPACER;
    }
    
    mylcd.LCDChar(numforlcdY.digits[d], col, YLCDPAGE);
    col += MCFFONTWIDTH + MCFFONTSPACER;
    
    int8_t thousandcheck = d + 1 - numforlcdY.dec_point_pos;
    
    if ( (thousandcheck % 3 == 0) && (thousandcheck != 0) )
    {
      col += MCFTHOUSANDSPACER;
    }     
  }
  
  if (numforlcdY.dec_point_pos == numforlcdY.num_digits && numforlcdY.show_dec_point)
  {
    mylcd.LCDDot(col, YLCDPAGE);
  }
  
  number_for_lcd numforlcdX;
  
  numforlcdX.digits[0] = 2; numforlcdX.digits[1] = 7; numforlcdX.digits[2] = 1;
  numforlcdX.digits[3] = 8; numforlcdX.digits[4] = 2; numforlcdX.digits[5] = 8;
  numforlcdX.digits[6] = 1; numforlcdX.digits[7] = 8; numforlcdX.digits[8] = 3; 
  numforlcdX.sign = 0;  // positive
  numforlcdX.dec_point_pos = 0;
  numforlcdX.num_digits = 9;
  numforlcdX.show_dec_point = true;
  
  col = 0;
  
  for (uint8_t d = 0; d < numforlcdX.num_digits; d++)
  {
    if (d == numforlcdX.dec_point_pos && numforlcdX.show_dec_point)
    {
      mylcd.LCDDot(col, XLCDPAGE);
      col += MCFDECPOINTWIDTH + MCFFONTSPACER;
    }
    
    mylcd.LCDChar(numforlcdX.digits[d], col, XLCDPAGE);
    col += MCFFONTWIDTH + MCFFONTSPACER;
    
    int8_t thousandcheck = d + 1 - numforlcdX.dec_point_pos;
    
    if ( (thousandcheck % 3 == 0) && (thousandcheck != 0) )
    {
      col += MCFTHOUSANDSPACER;
    }     
  }
  
  if (numforlcdX.dec_point_pos == numforlcdX.num_digits && numforlcdX.show_dec_point)
  {
    mylcd.LCDDot(col, XLCDPAGE);
  }
  
  delay(20000);
  
  // how to show hex numbers
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(0xA, 14*0, 0 * 3); // col 0-14, page 0-1
  mylcd.LCDChar(0xB, 14*1, 1 * 3); 
  mylcd.LCDChar(0xC, 14*2, 2 * 3); 
  mylcd.LCDChar(0xD, 14*3, 1 * 3); 
  mylcd.LCDChar(0xE, 14*4, 0 * 3); 
  mylcd.LCDChar(0xF, 14*5, 1 * 3); 
   
  delay(2000);
} // testStillNumbers()

// this shows how to change the contrast at run time to something other than the default
void testContrast() {
  for (byte c = 10; c <= 32; c++) {
    char hexString[3];
    char decString[4];    
    sprintf(hexString, "0x%02X", c);
    sprintf(decString, "%03d", c);    
    mylcd.LCDSetContrast(c);  

    //mylcd.LCDGotoXY(0, 0);
    mylcd.LCDString(hexString, 0, 0);
    //mylcd.LCDGotoXY(0, 1);
    mylcd.LCDString(decString, 0, 3);
   
    //mylcd.LCDChar(c % 10);
    //mylcd.LCDBitmap(0, 18, 14, 15, MiyoCalcFont_14x15[c]);
    //mylcd.LCDBitmap(0, 48, 169, 16, testImage);
    mylcd.LCDChar(c / 10, 0, 6);
    mylcd.LCDChar(c % 10, 14, 6);

    delay(1000);

    //mylcd.LCDFillScreen(0x00, 0); // Clear the screen
    //delay(50);    
  }
}




// example of how to print logos, etc, on the LCD. may want to use for indicators such as degrees vs radiant, and 'shift' mode (f, g, h)
void testBitmap() {
  //mylcd.LCDBitmap(0, 24 , 16, 16, smallImage);
  //mylcd.LCDBitmap(0,0,169,16,testImage);
  //mylcd.LCDBitmap(0,24,169,16,testImage);
  //mylcd.LCDBitmap(0,48,169,16,testImage);
  
  mylcd.LCDFillScreen(0x00, 0); // clear screen
//  mylcd.LCDBitmap(0,0,176,16,MiyoCalcTemp1);
//  mylcd.LCDBitmap(0,24,176,16,MiyoCalcTemp2);
//  mylcd.LCDBitmap(0,48,176,16,MiyoCalcTemp3);
  delay(5000);
  
  mylcd.LCDFillScreen(0x00, 0); // clear screen
//  mylcd.LCDBitmap(0,0,62,16,MiyoCalcTemp4);
  delay(5000);
    
}

// example of how to print letters on the LCD. Also shows where the numbers would start when displaying 6 numbers on screen. (e.g. a full stack with X, Z, Z, T, U and V)
void testSomeText() {
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(21, 0, 0); mylcd.LCDChar(21, 96, 0);  // TODO: make use of MCFLETOFFSET (replace hard-coded integer in first parameter by 'L' - MCFLETOFFSET, etc.)
  mylcd.LCDChar(10, 0, 3); mylcd.LCDChar(19, 96, 3);
  mylcd.LCDChar(23, 0, 6); mylcd.LCDChar(23, 96, 6);
  delay(5000);
}

void testSomeOtherText()
{
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDString("NOAH", 0, 0);
  mylcd.LCDString("AUDREY", 0, 3);
  mylcd.LCDString("LIAM", 0, 6);
  uint8_t txt1[] = {35, 26, 33}; 
  mylcd.LCDCharSeq(txt1, sizeof(txt1)/sizeof(txt1[0]), 96, 0);
  delay(5000);
}

// if 'off' button (combination) was pressed (can tell based on action), power down lcd and configure 'sleep mode' to powerdown
void power_down(uint8_t keycode)
{
  mylcd.FullLCDPowerDown(); 
  lcdon = false;
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
} // power_down()
