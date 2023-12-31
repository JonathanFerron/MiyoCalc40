#include "util.h"

/* 
helper function battery_voltage()

LiFePO4 10440 (AAA) 280mAh battery. Used capacity when cycling from 3.25V to 3.00V is about 140mAh.

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

Typical discharge when on and idle, with backlight off, with battery at 3.2V, is 265uA (mostly from LCD)
0.3mA / 300mAh = 0.001C

Power consumption when on and very active (continually pressing buttons very fast), with backlight off, is 5.7mA.

Power consumption in power down sleep mode is 7.3uA.

Avg current when in active use = 320uA, therefore runtime of about 140mAh / 0.320mA = 428h = 18 days

Avg runtime when in power down sleep mode = 140mAh / 0.0073uA = 19284h = 804 days = 26 months

Voltage measurement code:
 

*/

