#include <avr/io.h>
#include "spi.h"

void spi_init(void)
{ /* MOSI (PA4) and SCK (PA6) must be driven by us; MISO (PA5) stays input
     -- matrix.cpp already disables its digital input buffer as one of the
     two "unused" PORTA pins, since this driver never reads the LCD back. */
  PORTA.DIRSET = PIN4_bm | PIN6_bm;

  SPI0.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm;    /* mode 0; SSD: PA7 (LCD /CS)
                                                  is plain GPIO, not managed
                                                  by the SPI peripheral */
  SPI0.CTRLA = SPI_MASTER_bm | SPI_PRESC_DIV4_gc | SPI_ENABLE_bm;
                                               /* master, CLK_PER/4 = 6MHz,
                                                  MSB-first (DORD left 0) */
} // spi_init

void spi_disable(void)
{ SPI0.CTRLA &= ~SPI_ENABLE_bm;
} // spi_disable

uint8_t spi_transfer(uint8_t data)
{ SPI0.DATA = data;
  while (!(SPI0.INTFLAGS & SPI_IF_bm))
    ;
  return SPI0.DATA;
} // spi_transfer
