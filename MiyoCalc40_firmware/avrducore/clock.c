#include <avr/io.h>
#include "clock.h"
#include "ccp.h"

void clock_init(void)
{ /* Select OSCHF as main clock source, no clock output on pin */
  ccp_write_ioreg((void*)&CLKCTRL.MCLKCTRLA,
                  CLKCTRL_CLKSEL_OSCHF_gc);

  /* Prescaler disabled — run OSCHF directly at 24 MHz */
  ccp_write_ioreg((void*)&CLKCTRL.MCLKCTRLB,
                  0);

  /* OSCHF: 24 MHz, AUTOTUNE left off (bit 0, defaults to 0 — this part's
     autotune references XOSC32K, not USB SOF as on the DU series; we have
     neither a crystal nor USB here, so it stays disabled either way). */
  ccp_write_ioreg((void*)&CLKCTRL.OSCHFCTRLA,
                  CLKCTRL_FRQSEL_24M_gc);

  /* Wait for OSCHF to lock before any peripheral init */
  while(!(CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm))
    ;

  /* CPUINT: default priority, no round-robin, no compact vectors */
  ccp_write_ioreg((void*)&CPUINT.CTRLA, 0);
} // clock_init
