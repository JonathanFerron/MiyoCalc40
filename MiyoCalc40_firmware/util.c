// config mode code for key debouncing delays, and screen contrast (delegate most of this to the lcd.c file), etc. should be hosted here

#include "util.h"

/* 
helper function battery_voltage()
	see openrpncalc main.c file

LiFePO4 10440 (AAA) 280mAh to 300mAh battery

Recharge battery to 90%: 3.25V
When depleted at 10%: 3.00V
Don't ever let battery go lower than 2.6V
Discharge curve fairly flat from 3.25V to 3.15V
From 3.4V (100% SOC, 300mAh) to 3.25V (90% SOC, 270mAh), convex curve (from 45 degree to flat)
From 3.15V (20% SOC) to 3.0V (10% SOC, 30mAh), concave curve (from flat to 45 degree)
From 3.0V to 2.6V (0% SOC), still concave curve, but rapidly decreasing (from 45 degree to almost 90 degree pointing down)
Add a discharge curve plot on the silkscreen (base plate, along with 'cheat sheet' to use calculator)
Nominal voltage of LiFePO4 battery: 3.2V
MCU voltage range: 1.8V to 5.5V
LCD voltage tolerance range: 3.0V to 3.6V (3.3V ideal): actually shuts down at 2V

store battery ideally at 3.2V (50% of SOC)

Charge battery using CCCV: 
first cap current at 56mA to 90mA (0.2C to 0.3C)
then cap charging voltage at 3.5V
Stop charging when battery voltage reaches 3.25V, or when charging current drops to 15mA (0.05C), whichever comes first
should take about 3 to 5 hours to charge battery from 10% SOC to 90% SOC (240mAh).

Typical discharge when on will likely be around 5mA (mostly from LCD, really): or measure when using calc with a multimeter
5mA / 300mAh = 0.02C

Voltage measurement code:
 
For Atmega328p, this would be
use ADC with fixed reference. Can measure Reference Voltage (1.1V) against VCC. 
ADC = (Vin * 1024) / Vref    =>    ADC = (1.1v * 1024) / Vcc    =>    Vcc = (1.1v * 1024) / ADC
ADMUX = 0b00100001
ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0)
delay 1ms
...
see wp.josh.com, battery fuel gauge with zero parts and zero pins on avr

on avr-da:
* ADC and VREF: ADC0
* ADCenable: CTRLA bit 0
* single ended conversion: CTRLA bit 5
* 10 bit conversion: CTRLA bit 3:2
* ADC0.RES = RES = VAinp * 1024 / VADCREF  (VADCREF = VREF.ADC0REF). this will yield an int from 0 to 1023 inclusive
* 
* selection of ADC input: MUXPOS
* slightly preferable for VDD around 3V or 3.1V: 2.5V reference
* pick ADCREF = VDD and DACREF = 2.5V
* pick DACREF0 = VREF.DAC0REF
* 
* VREF.ADC0REF.REFSEL = 4V096, value 0x2
* VREF.DAC0REF.REFSEL = VDD, value 0x5


*/

