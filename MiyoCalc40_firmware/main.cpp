 /* Other notes:

	use static vars if necessary
  
  calc, config and programming mode: should be stored in a global status field that can be set and checked, could be an enum
  f, g, h shift modifiers: should each be a bool global variable that can be set and checked
  config screen: should be stored in a global status fiels as well that can be set and checked, could be an enum (only relevant in 'config' mode)
  
  see repocalc, openrpncalc, dcalc (main.c) for examples
        H:\My Drive\MiyoCalc40\resources\openrpncalc
        H:\My Drive\MiyoCalc40\resources\repocalc
        H:\My Drive\MiyoCalc40\resources\dcalc-2.12
       
  Open RPN Calc:
    helper function switch_to_input() to switch input pins to simple input pull-up mode (no interrupt)

    helper function put_to_sleep() to switch input pins back to interupt mode and put mcu in sleep mode
      set all input pins to pullup with interupt falling mode
      call enable_sleep_on_exit() helper function: Set SLEEPONEXIT bit. When this bit is set, the processor 
            re-enters SLEEP mode when an interruption handling is over. look at avr matrix scanning code and 
            avr128da28 datasheet to figure out how to do that with an avr-da chip
      call suspend_tick() helper function: Suspend 'Tick' interupt
            Otherwise the Systick interrupt will wake up the device within 1ms. look up how to do that with avr-da.
      call enterstop2mode() helper function: Enter in Low power Stop 2. look up how to do that with avr-da.
  
  see following likes for useful info:
    dxcore/megaavr/extras/ioheaders/readme.md
    dxcore/megaavr/extras/DA28.md
    * 
  
  MCU draws about 6.0mA at 3.2V when actively running at 24 MHz
     
*/

#include "matrix.h"
#include "lcd.h"
#include "fonts.h"
#include "main.h"

#define VbiasPOT 0x0C // Default constrast: 0x0A to 0x20 tend to work well

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
  
  // setupCalc(); this method should be implemented in the calc.c and calc.h files
  //setup calc: see repocalc, openrpncalc, dcalc for examples
   
  setupLCD();
  lcdon = true;
  setupBacklight(); 
  
 
} // setup()

void setupMCU()
{
  // look into turning off TCD0 to save power (call takeoverTCD0() perhaps )
}

void setupLCD()  // this should be moved to the lcd file
{
  mylcd.LCDbegin(VbiasPOT); // initialize the LCD
  mylcd.LCDFillScreen(0x00, 0); // clear screen  
  delay(50);
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
go in power-down sleep mode (set power down sleep mode and enable sleep, enable bothedges interrups for all column pins) (unless we're in 'off' state, in which case 
may be able to only enable interupt on one pin)): see dxcore/megaavr/extras/powersave.md, do not use avr/power.h
		
  upon wake interupt (port isr wake-up button press wakes up the mcu from sleep)					
    debounce 3ms to 20ms (10 times 2ms) using PIT (periodic interupt timer, programmed to trigger an interrupt every millisecond, lowpower_delay_ms function, set interrupt 
    to fire every 32 RTC clock cycles and enable PIT, enable PIT interrupts), validating logic low level on a column each time: btn_debounce() function					
    
    if logic low level still detected in a column					
      then scan keys (scan matrix) via function call: scankb()			
      
      if calc is off, power up lcd if 'on' button was pressed (key position for this can be stored in a separate global variable used directly in 'main'), 
        record calc_off variable = false
      else (calc is on)
        if in mem store or recall mode:
          call mem store or recall with key pos, should have a max of 38 possible var 
        else
          with key position (from scankb), lookup (cards.c) function pointer and keycode (action struct)
          
          if in 'calc mode'
            if 'off' button (combination) was pressed (can tell based on the keycode alone), power down lcd and calc, record calc_off variable = true         
            else
              process action via function call (from function pointer): from 'action' struct from 'calc' (if in calc mode) or 'config' (if in config mode)
                provide it the keycode that was also looked up
                action in program mode, when a key is pressed to be recorded, is a function that will call another function with a keycode to log/record it					
            end if
          end if
        end if
      endif

      refresh lcd when necessary (typically after calculation has been processed and before going back to sleep)  					
      
      inner loop begin (idle until key is released):					
        if logic low level on any column still					
          then power down sleep					
            port isr wake-up: button is released					
        end if					
      end inner loop					
      may want to debouce here as well (openrpncalc uses 10ms)					
    end if					
 */
void loop() 
{  
  // scan keys
  scanKB();
 
  // if non-null keypos:
  if (keypos_r != 0xff)
  {
    
    // lookup function pointer+keycode (action struct): this should include re-setting the keypos to 'null' once the action is obtained
  
    // process action via function call: from 'action' struct from 'calc', call the function and provide it the keycode (not the keypos) that was also looked up. 
  
    // refresh lcd
    mylcd.LCDFillScreen(0x00, 0); // clear screen
    mylcd.LCDChar('R' - MCFLETOFFSET, 14*0, 0*3);  // R
    mylcd.LCDChar(keypos_r, 14*1, 0 * 3);
    mylcd.LCDChar('C' - MCFLETOFFSET, 14*3, 0*3); // C 
    mylcd.LCDChar(keypos_c, 14*4, 0 * 3);  
    
    
  } // end if non-full keypos 
  
} // loop()

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

// old loop() code for testing
void loopOld() {
  //testContrast();
  //testStillNumbers();
  //testSomeText();
  testSomeOtherText();
  
  // test backlight
  //digitalWrite(PIN_PA1, HIGH);
  //delay(10000);
  //digitalWrite(PIN_PA1, LOW);
//  testBitmap();
} // loopOld()

// shows how we would print numbers when displaying 3 elements of the stack (X at bottom, then Y, then Z on top)
void testStillNumbers()
{
  mylcd.LCDFillScreen(0x00, 0); // clear screen
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
  
  delay(5000);
  
  // how to show hex numbers
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(0xA, 14*0, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xB, 14*1, 1 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xC, 14*2, 2 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xD, 14*3, 1 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xE, 14*4, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xF, 14*5, 1 * 3); // col 0-11, page 0-1
   
  delay(5000);
}

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
  uint8_t txt1[] = {35, 26, 33}; mylcd.LCDCharSeq(txt1, sizeof(txt1)/sizeof(txt1[0]), 96, 0);
  delay(5000);
}
