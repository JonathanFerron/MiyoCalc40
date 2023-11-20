/*
config mode code for:
- battery voltage display
- screen contrast (delegate most of this to the lcd.c file)
- backlight front end (delegate most to backlight.c, use left & right for off and on)
- max stack size (4 to 8) and mode: fixed depth, aka traditional hp rpn, or 'normal' stack that grows and shrinks from a minimum size of 1
- 1 or 2 column
- switching keymap (card, require user to key in card number from 0 to 9 after)
- import & export programs from/to progmem (then user key any of 36 program key, when keys for unique program are pressed)
- key debouncing delays / parameters
- soft reset

Config could be stored in eeprom.
 
Implement here the LCDDrawConfigScreen() function.

LCDDrawConfigScreen() function :
  switch/case based on 'current cfg screen' and call specialized 'LCDDrawConfigXYZ()' funtions, e.g. LCDDrawConfigBattVolt()

when 'backlight' button is keyed from main cfg screen:
  toggle 'current cfg screen' to 'backlight'
  draw config screen

when 'battvolt' button is keyed from main cfg screen:
  toggle 'current cfg screen' to 'battvolt'
  configure peripherals required to calculate battery voltage
  draw config screen

when up, down, left and right arrow (special KCC_ non-programmable key codes, use separate defines, e.g. KCC_UP) are pressed in config mode, 
call "config_adjust(keycode)" function

config_adjust:
  switch (current cfg screen)
    case backlight
      call adj_bl(keycode)
      draw cfg screen


config_cancel_exit() function, called when cancel/exit button is pressed
  switch (current cfg screen)
    case battvolt
      turn off peripherals that were enabled for battery voltage
      chg current cfg screen to main screen
    case maincfgscreen
      get out of config mode, chg mode to calculator mode


 */

#include "config.h"
