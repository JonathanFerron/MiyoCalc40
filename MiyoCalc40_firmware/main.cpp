/*
   To build and flash the firmware: `make` then `make flash` from MiyoCalc40_firmware/ (see
   CLAUDE.md and makefile for the full make target list).

   main: main program loop

   matrix: matrix scanning code

   cards: contains mapping from matrix key positions to 'actions' for each 'mode'

   calc: number crunching mode

   lcd: to control lcd

   fonts: fonts bitmaps

   programming: programming mode

   config: configuration mode
     util: to display battery voltage, etc

   backlight: to turn on/off backlight and set pwm duty cycle

   avrducore: clock/CCP/fuses/GPIO/SPI0
*/

/* Other notes:

  MCU draws about 6.0mA at 3.2V when actively running at 24 MHz

*/

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "matrix.h"
#include "lcd.h"
#include "main.h"
#include "cards.h"
#include "calc.h"
#include "util.h"
#include "clock.h"
#include "gpio.h"
#include "backlight.h"

#define LCD_Default_Contrast 0x0C // Default LCD constrast: 0x0A to 0x20 tend to work well.

// GPIO 5-wire SPI interface

#define LCD_CS GPIO_PIN(PORTA, 7)  // AVR pin connected to LCD CS pin
#define LCD_CD GPIO_PIN(PORTA, 2) // AVR pin connected to LCD CD pin
#define LCD_RST GPIO_PIN(PORTA, 3) // AVR pin connected to LCD Reset pin

// Global variables
ERM19264_UC1609_T  mylcd(LCD_CD, LCD_RST, LCD_CS); // construct object using hardware SPI: CD, RST, CS

int lcdon;  // to track if lcd is turned on or not, to do: change this to 'calcon'

int main()
{ clock_init(); // avrducore/clock.c: 24MHz OSCHF, no prescaler, CPUINT defaults

  /* Insert here any code that needs to run before interrupts are enabled but after all other core initialization. */
  sei();  // enable interrupts (could consider turning on interupts only after the setup code below, if it matters at all).

  setupMCU();
  setupMatrix();

  setupLCD();

  backlight_init();
  setupBattVoltMonitor();

  calc_init();

  for(;;)
    loop();
}

void setupMCU()
{ // ADC/AC/VREF setup for battery voltage monitoring lives in setupBattVoltMonitor() (util.cpp),
  // called from main(). The ADC itself is left disabled there; it's only enabled for the
  // duration of a reading, by read_batt_voltage_mv().

  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_enable();
  /* Enable BOTHEDGES interrupts for columns */
  PORTC.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTC.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTC.PIN2CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTC.PIN3CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTD.PIN4CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTD.PIN5CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTD.PIN6CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTD.PIN7CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTF.PIN0CTRL |= PORT_ISC_BOTHEDGES_gc;
  PORTF.PIN1CTRL |= PORT_ISC_BOTHEDGES_gc;
}

ISR(PORTC_PORT_vect)
{ /* Clear interrupt flag */
  VPORTC.INTFLAGS = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
}

ISR(PORTD_PORT_vect)
{ /* Clear interrupt flag */
  VPORTD.INTFLAGS = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
}

ISR(PORTF_PORT_vect)
{ /* Clear interrupt flag */
  VPORTF.INTFLAGS = PIN0_bm | PIN1_bm;
}


void setupLCD()
{ mylcd.LCDbegin(LCD_Default_Contrast); // initialize the LCD
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  _delay_ms(50);
  lcdon = true;
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
bool press_valid = false;
void loop()
{ // go in sleep mode
  sleep_cpu();

  // debounce 10 * 2ms = 20ms
  press_valid = true;
  for(uint8_t i = 0; i < 10; i++)
  { // initialize before scanning
    keypos_c = 0xff;
    keypos_r = 0xff;
    scanKB(); // scan kb
    /* If no button is pressed, flag press as non-valid */
    if(keypos_c == 0xff && keypos_r == 0xff)
    { press_valid = false;
      break;
    }
    //lp_delay_ms(2);
    _delay_ms(2);  // to-do: chg this to PIT based delay to save power
  }  // end for loop for debouncing

  // if key press is valid, then move on to scan and process key press
  if(press_valid)
  { // scan keys
    scanKB();

    // if lcd is off, power it up if 'on' button was pressed
    if(keypos_r == ONOFFKEYPOS_R && keypos_c == ONOFFKEYPOS_C && !lcdon)
    { setupLCD();
      backlight_resume();
      lcdon = true;
      if(input.started)
        LCDDrawInput();
      else
        LCDDrawStackAndMem();

      set_sleep_mode(SLEEP_MODE_STANDBY);
      shift = baseLayer;
      keypos_c = 0xff;
      keypos_r = 0xff; // reset keypos to 'null' after action is obtained
    } // 'power on' lcd

    // if non-null keypos and calc is on:
    if(lcdon && keypos_r != 0xff)
    { // if in mem store, recall mode or clear mode
      if(mem_recall_mode || mem_store_mode || mem_clear_mode)
      { if(mem_recall_mode)
        { // process action
          // call apply_memory_rcl(uint8_t r, uint8_t c) to recall the variable to X
          apply_memory_rcl(keypos_r, keypos_c);
          keypos_c = 0xff;
          keypos_r = 0xff; // reset keypos to 'null' after action is obtained

          // reset mode to false after action has been processed
          mem_recall_mode = false;
        }
        else if(mem_store_mode)
        { // process action
          apply_memory_sto(keypos_r, keypos_c);
          keypos_c = 0xff;
          keypos_r = 0xff; // reset keypos to 'null' after action is obtained

          // reset mode to false after action has been processed
          mem_store_mode = false;
        }
        else // we're in mem clear mode
        { // process action
          apply_memory_clr(keypos_r, keypos_c);
          keypos_c = 0xff;
          keypos_r = 0xff; // reset keypos to 'null' after action is obtained

          // reset mode to false after action has been processed
          mem_clear_mode = false;
        }
        LCDDrawCalcStatus();
      }
      else // not in mem store, recall, or clear mode
      { // lookup function pointer+keycode (action struct)
        action current_action = keytoaction();
        // refresh lcd : this will become obsolete since we'll refresh the lcd from within the actions, when necessary
        /*
          mylcd.LCDFillScreen(0x00, 0); // clear screen
          mylcd.LCDChar('R' - MCFLETOFFSET, 14*0, 0*3);  // R
          mylcd.LCDChar(keypos_r, 14*1, 0 * 3);
          mylcd.LCDChar('C' - MCFLETOFFSET, 14*3, 0*3); // C
          mylcd.LCDChar(keypos_c, 14*4, 0 * 3);
        */

        keypos_c = 0xff;
        keypos_r = 0xff; // reset keypos to 'null' after action is obtained

        // process action via function call: from 'action' struct, call the function and provide it the keycode that was also looked up.
        current_action.fct(current_action.keycode);

        // if (current shift is not base layer && keycode of function that was just executed is not a layer shift keycode that corresponds to the current layer), then move back to base layer
        // config mode has no shift layers of its own, and enter_shift_base() would repaint the
        // calc status indicators over whatever config screen is currently displayed, so skip
        // all of this while in config mode.
        if(current_calc_prog_config_mode != config_mode)
        { if(!((current_action.mnemonic[0] == ACT_SHFT_F.mnemonic[0] && current_action.mnemonic[1] == ACT_SHFT_F.mnemonic[1] && shift == fLayer) ||
               (current_action.mnemonic[0] == ACT_SHFT_G.mnemonic[0] && current_action.mnemonic[1] == ACT_SHFT_G.mnemonic[1] && shift == gLayer) ||
               (current_action.mnemonic[0] == ACT_SHFT_H.mnemonic[0] && current_action.mnemonic[1] == ACT_SHFT_H.mnemonic[1] && shift == hLayer)))
          { if(lcdon)
              enter_shift_base(KC_NOP);
            else
              shift = baseLayer;
          }
        }
        else
          shift = baseLayer;
      }

      do
      { keypos_c = 0xff;
        keypos_r = 0xff;
        scanKB();
      }
      while(keypos_c != 0xff || keypos_r != 0xff);

    } // end processing of non-null keypos
  } // end processing of valid key press

} // loop()

// Battery voltage measurement (setupBattVoltMonitor() / read_batt_voltage_mv()) now lives in
// util.cpp, wired up to the 'config' mode battery-voltage screen (config.cpp). See util.cpp for
// the derivation of the VDD-in-millivolts calculation and the peripheral setup this depends on.

// if 'off' button (combination) was pressed (can tell based on action), power down lcd and configure 'sleep mode' to powerdown
void power_down(__attribute__((unused)) uint8_t keycode)
{ mylcd.FullLCDPowerDown();
  backlight_suspend(); // see backlight.h: TCA0 must not be left running into Power-Down sleep
  lcdon = false;
  //LCDDrawStackAndMem();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
} // power_down()
