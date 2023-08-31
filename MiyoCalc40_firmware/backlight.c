/* 
* LED Backlight Code
* PWM on PIN_PA1
* 2.54V voltage drop
* 3.2 VCC from battery (typical)
* 1800ohm resistor -> 0.37mA = 370 uA current draw if run with 100% duty cycle
* with a multimeter, measured 369 uA current draw (100% duty cycle) at 3.2V
* 
* We use the TCA0 timer with PWM on pin A1
* 
* To control frequency of the PWM, see example 2 in 'taking over TCA0' documentation from DxCore.
* 
* For LED dimming, PWM frequency should be at least 300Hz.
* 
* DxCode analogwrite() : 0 means 'off', and 254 means 'always on'
* 
*/

#include "backlight.h"


