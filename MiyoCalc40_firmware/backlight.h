// backlight.h

/* PWM backlight driver, TCA0 split 8-bit mode on PIN_PA1 (WO1). Fixed ~1.47kHz
   (24MHz/DIV64/255 steps, matching what DxCore's init_TCA0() used to set up before the
   port off Arduino/DxCore -- see the removed setupBacklight() comment in main.cpp's history).

   Brightness is addressed as an index into a 20-entry perceptual table (see backlight.cpp),
   not a raw PWM duty -- same approach as KeyDU-Citrouille90's led.c, just fewer notches since
   this is a rarely-used backlight rather than an always-visible indicator. Index 0 (off) and
   the last index (255, full on) are both plain GPIO, no timer involved. Only the 18 notches in
   between spin up TCA0, and only then is RUNSTDBY set on it, so the PWM keeps running through
   SLEEP_MODE_STANDBY between keypresses -- TCA halts in Standby by default otherwise
   (hardware-verified against DS40002183C SS21.3.6 "TCA - Sleep Mode Operation" and Table 12-2).
   Since the backlight is expected to be used rarely, this keeps the RUNSTDBY power cost scoped
   to only the time it's actually on and dimmed. */
#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include <stdbool.h>
#include <stdint.h>

#define BACKLIGHT_TABLE_SIZE 20 // index 0 = off, index 19 = full brightness (255)

#ifdef __cplusplus
extern "C"
{
#endif

void backlight_init(void);
void backlight_set(uint8_t index);
void backlight_step(bool dir); // true = one notch brighter, false = one notch dimmer
uint8_t backlight_get(void);   // current table index, 0..BACKLIGHT_TABLE_SIZE-1

// Force the physical output off without forgetting the current index (pairs with
// backlight_resume()) -- for the full soft-power-off path. TCA0 halts unpredictably mid-PWM-cycle
// in SLEEP_MODE_PWR_DOWN regardless of RUNSTDBY (DS40002183C Table 12-2/SS21.3.6), so a dimmed
// backlight left running into power-down freezes at an arbitrary on/off/dim level; suspending it
// first avoids that instead of relying on the sleep mode to do it cleanly.
void backlight_suspend(void);
void backlight_resume(void);

#ifdef __cplusplus
}
#endif

#endif /* BACKLIGHT_H */
