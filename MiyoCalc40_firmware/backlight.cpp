/*
  LED Backlight Code
  PWM on PIN_PA1
  2.54V voltage drop
  3.2 VCC from battery (typical)
  1800ohm resistor -> 0.37mA = 370 uA current draw if run with 100% duty cycle
  with a multimeter, measured 369 uA current draw (100% duty cycle) at 3.2V

  We use the TCA0 timer with PWM on pin A1, split 8-bit mode (LPER/LCMP1), routed to port A so
  WO1 lands on PA1. See backlight.h for the RUNSTDBY / Standby-sleep rationale.
*/

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "backlight.h"
#include "gpio.h"

/* Perceptual brightness table: I_n = round(8 * 1.212^(n-1)) for n=1..18, endpoints pinned to
   0 (off) and 255 (full) so the table always reaches both extremes exactly regardless of
   rounding. Same Weber-fraction geometric approach as KeyDU-Citrouille90's led.c (ratio 1.162
   over 25 entries there); this table just covers the same 0-255 range in fewer notches (20 vs
   25), so the ratio -- and therefore the perceived jump per notch -- is correspondingly larger. */
static const uint8_t backlight_table[BACKLIGHT_TABLE_SIZE] PROGMEM =
{ 0,
  8,  10,  12,  14,  17,  21,  25,  31,  37,
  45,  55,  66,  80,  97, 118, 143, 174, 210, 255
};

static const gpio_pin_t BACKLIGHT_PIN = GPIO_PIN(PORTA, 1); // LCD LED backlight pin, TCA0 WO1

static uint8_t backlight_index;

static void backlight_write(uint8_t duty)
{ if(duty == 0 || duty == 255)
  { TCA0.SPLIT.CTRLA = 0;                     // stop the timer, drop RUNSTDBY with it
    TCA0.SPLIT.CTRLB &= ~TCA_SPLIT_LCMP1EN_bm; // hand PA1 back to plain GPIO control
    if(duty == 0)
      GPIO_LOW(BACKLIGHT_PIN);
    else
      GPIO_HIGH(BACKLIGHT_PIN);
  }
  else
  { TCA0.SPLIT.LPER = 0xFE;  // 255-step period -> ~1.47kHz at CLK_PER/64 (24MHz)
    TCA0.SPLIT.LCMP1 = duty;
    TCA0.SPLIT.CTRLB |= TCA_SPLIT_LCMP1EN_bm; // PA1 now driven by the compare match, not GPIO
    TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV64_gc | TCA_SPLIT_RUNSTDBY_bm | TCA_SPLIT_ENABLE_bm;
  }
} // backlight_write

void backlight_init(void)
{ GPIO_SET_OUTPUT(BACKLIGHT_PIN);
  PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTA_gc; // route TCA0 to port A: WO1 is PA1
  TCA0.SINGLE.CTRLD = TCA_SINGLE_SPLITM_bm;  // split mode (same physical bit as TCA0.SPLIT.CTRLD)

  backlight_index = 0;
  GPIO_LOW(BACKLIGHT_PIN); // start off; TCA0 stays disabled (CTRLA.ENABLE resets to 0)
} // backlight_init

void backlight_set(uint8_t index)
{ if(index >= BACKLIGHT_TABLE_SIZE)
    index = BACKLIGHT_TABLE_SIZE - 1;
  backlight_index = index;
  backlight_write(pgm_read_byte(&backlight_table[backlight_index]));
} // backlight_set

void backlight_step(bool dir)
{ if(dir)
  { if(backlight_index < BACKLIGHT_TABLE_SIZE - 1)
      backlight_index++;
  }
  else
  { if(backlight_index > 0)
      backlight_index--;
  }
  backlight_write(pgm_read_byte(&backlight_table[backlight_index]));
} // backlight_step

uint8_t backlight_get(void)
{ return backlight_index;
} // backlight_get

void backlight_suspend(void)
{ TCA0.SPLIT.CTRLA = 0;
  TCA0.SPLIT.CTRLB &= ~TCA_SPLIT_LCMP1EN_bm;
  GPIO_LOW(BACKLIGHT_PIN);
} // backlight_suspend

void backlight_resume(void)
{ backlight_write(pgm_read_byte(&backlight_table[backlight_index]));
} // backlight_resume
