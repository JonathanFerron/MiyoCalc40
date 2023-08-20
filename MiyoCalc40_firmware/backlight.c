/* 
* LED Backlight Code
* PWM on PIN_PA1
* 2.54V voltage drop
* 3.2 VCC from battery (typical)
* 1800ohm resistor -> 0.37mA = 370 uA current draw if run with 100% duty cycle
* with a multimeter, measured 369 uA current draw (100% duty cycle)
*/

#include "backlight.h"


