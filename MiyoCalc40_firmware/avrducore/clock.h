#ifndef CLOCK_H
#define CLOCK_H

/* System clock: 24 MHz OSCHF, no prescaler. Matches the "clock=24internal"
   board option the Arduino/DxCore build used, and is the max rated speed
   for AVR128DA28. Verified against DxCore's wiring.c: it writes
   OSCHFCTRLA = (0x09 << 2) for F_CPU == 24000000, byte-identical to
   avr-libc's CLKCTRL_FRQSEL_24M_gc used below. */
#ifndef F_CPU
  #define F_CPU 24000000UL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Initialise CLKCTRL and CPUINT. Blocks until OSCHF is stable. */
void clock_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_H */
