#ifndef MAIN_H
#define MAIN_H

#include "lcd.h"

extern ERM19264_UC1609_T mylcd;

void setup();
void loop();

void testStillNumbers();
void testStillNumbers2();
void testSomeText();
void testSomeOtherText();
void testBatteryVoltage();

void setupMCU();
void setupLCD();
void setupBacklight();

void loopTests();
void loopTestBacklightPWM();

#endif
