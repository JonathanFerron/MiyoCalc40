#ifndef MAIN_H
#define MAIN_H

#include "lcd.h"

extern ERM19264_UC1609_T mylcd;

void loop();

void setupMCU();
void setupLCD();

#endif
