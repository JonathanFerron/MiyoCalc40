/*
config mode code for:
- key debouncing delays / parameters
- soft reset
- screen contrast (delegate most of this to the lcd.c file)
- backlight front end (delegate most to backlight.c) 
- max stack size (3 to 8) and mode: fixed depth, aka traditional hp rpn, or 'normal' stack that grows and shrinks from a minimum size of 1
- switching keymap (card)
- battery voltage display
- import & export programs from/to progmem
- etc

Mode should be accesses via g or h + other key, this needs to be an action (not programmable, so with KC_NOP) in the 'calc card'. 

Config could be stored in eeprom.
 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

#include "config.h"
