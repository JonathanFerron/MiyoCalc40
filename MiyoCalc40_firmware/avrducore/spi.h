#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/* SPI0 master driver, hardware SPI0 on its default PORTA pin mapping
   (MOSI=PA4, MISO=PA5, SCK=PA6). Chip select is NOT managed here -- each
   peripheral (see lcd.h) drives its own CS pin via gpio.h around the
   transfers it needs, matching how the LCD driver already wraps every
   transaction in UC1609_CS_SetLow/High.

   Fixed configuration: MSB-first, SPI mode 0, CLK_PER/4. At F_CPU=24MHz
   that's 6MHz -- matching what lcd.h previously requested from Arduino's
   SPI library (SPI_FREQ 8MHz cap, fastest divider <= 8MHz available on a
   24MHz clock). Write-only: the LCD is never read back, so spi_transfer()
   is a synchronous full-duplex clock-out that the caller discards the
   received byte from, same as the old `(void)SPI.transfer(byte)`. */
#ifdef __cplusplus
extern "C" {
#endif

void spi_init(void);
void spi_disable(void);
uint8_t spi_transfer(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif /* SPI_H */
