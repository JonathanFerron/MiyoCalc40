/*
config mode code for key debouncing delays, and screen contrast (delegate most of this to the lcd.c file), etc. should be hosted here

allow switching keymap (card) on the fly from config menu

general config, eg contrast, debouncing parameters, soft reset, access via g or h + other key. config could be stored in eeprom
* 
* Add ability to change in config the max stack depth (3 to 8) and whether stack should have fixed depth (traditional hp rpn) or be a 'normal' stack that
grows and shrinks from a minimum size of 1 (X only).

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
