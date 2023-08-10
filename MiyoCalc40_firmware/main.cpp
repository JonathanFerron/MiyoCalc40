/* Combined Main Pseudo Code: 
 Main:					
  Init MCU: 					
    active mode					
    set-up column pins as input with pull-up and row pins as output low					
    set unused pins as input pull-up, and disable the digital input buffer on the unused pins (porta.pin0ctrl = port_pullupen_bm | port_isc_input_disable_gc)					
    set 'calcoff' = false

  setup calc: see repocalc, openrpncalc, dcalc for examples
  
  setupLCD(): already done
					
  main Loop Begin:					
    go in power-down sleep mode (set power down sleep mode and enable sleep, enable bothedges interrups for all column pins) (unless we're in 'off' state, in which case 
    may be able to only enable interupt on one pin))					
      upon wake interupt (port isr wake-up button press wakes up the mcu from sleep)					
        debounce 3ms to 20ms (10 times 2ms) using PIT (periodic interupt timer, programmed to trigger an interrupt every millisecond, lowpower_delay_ms function, set interrupt 
        to fire every 32 RTC clock cycles and enable PIT, enable PIT interrupts), validating logic low level on a column each time: btn_debounce() function					
        
        if logic low level still detected in a column					
          then scan keys (scan matrix) via function call: scankb() from matrix.c					
					
          if calc is off, power up lcd if 'on' button was pressed (key position for this can be stored in a separate global variable used directly in 'main'), 
            record calc_off variable = false
          else (calc is on)
            if in mem store or recall mode:
              call mem store or recall with key pos, should have a max of 38 possible var 
            else
              with key position (from scankb), lookup (cards.c) function pointer (action)
              process action via function call: from 'action' struct from 'calc' (if in calc mode) or 'config' (if in config mode), 
              action in program mode, when a key is pressed to be recorded, is a function that will call another function with a keycode to log/record it					
              if in 'calc mode'
                if 'off' button (combination) was pressed (can tell based on the keycode alone), power down lcd and calc, record calc_off variable = true         
                else, process keycode, go to 24 mhz to execute calculations: call the function via the function pointer, and provide it the keycode that was also looked up
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
  end main loop 
 */
 
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
     
*/

#include "ERM19264_UC1609_T.h"
#include "fonts.h"
#include "main.h"

#define VbiasPOT 0x0C // Constrast: 0x0A to 0x20 tend to work well

// GPIO 5-wire SPI interface

/* //uncomment for arduino uno
#define CS 10  // GPIO pin number pick any you want
#define CD  9 // GPIO pin number pick any you want 
#define RST 8 // GPIO pin number pick any you want
// GPIO pin number SDA(UNO 11) , HW SPI , MOSI
// GPIO pin number SCK(UNO 13) , HW SPI , SCK
*/

// uncomment for avr-da
#define CS PIN_PA7  // GPIO pin number pick any you want
#define CD PIN_PA2 // GPIO pin number pick any you want 
#define RST PIN_PA3 // GPIO pin number pick any you want

// Global variables
ERM19264_UC1609_T  mylcd(CD, RST, CS); // construct object using hardware SPI: CD, RST, CS

int main() {
//  onBeforeInit(); // Empty callback called before init but after the .init stuff. First normal code executed
  init(); // Interrupts are turned on just prior to init() returning.
//  initVariant();

  /* Insert here any code that needs to run before interrupts are
   * enabled but after all other core initialization. */
  sei();  // enable interrupts (by default, comment out if not desired).
  
  setup();
  
  for (;;) {
    loop();
  }
}

void setup() {
  //pinMode(PIN_PA7, OUTPUT);  // this was the test piece of code to blink a led on pin A7
  setupLCD();
  
  pinMode(PIN_PA1, OUTPUT); // to test LCD LED Backlight
}

void setupLCD()
{
  mylcd.LCDbegin(VbiasPOT); // initialize the LCD
  mylcd.LCDFillScreen(0x00, 0); // clear screen  
  delay(50);
}

void loop() {
  /*
  digitalWrite(PIN_PA7, HIGH);  // This was the test piece of code to blink a led on pin A7
  delay(500);
  digitalWrite(PIN_PA7, LOW);
  delay(500);
  */
  //testContrast();
  testStillNumbers();
  testSomeText();
  
  // test backlight
  digitalWrite(PIN_PA1, HIGH);
  delay(10000);
  digitalWrite(PIN_PA1, LOW);
//  testBitmap();
}

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
  
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(0xA, 14*0, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xB, 14*1, 1 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xC, 14*2, 2 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xD, 14*3, 1 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xE, 14*4, 0 * 3); // col 0-11, page 0-1
  mylcd.LCDChar(0xF, 14*5, 1 * 3); // col 0-11, page 0-1
   
  delay(5000);
}

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

void testSomeText() {
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDChar(21, 0, 0); mylcd.LCDChar(21, 96, 0);  // TODO: make use of MCFLETOFFSET
  mylcd.LCDChar(10, 0, 3); mylcd.LCDChar(19, 96, 3);
  mylcd.LCDChar(23, 0, 6); mylcd.LCDChar(23, 96, 6);
  delay(5000);
}
