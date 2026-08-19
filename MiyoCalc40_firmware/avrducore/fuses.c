#include <avr/io.h>

/* AVR128DA28 fuse configuration.

   These are consumed by avr-objcopy at link time and written to the .fuses
   section in the ELF. avrdude programs them into the device fuse row
   separately from flash.

   Every field below was cross-checked two ways against
   /usr/lib/avr/include/avr/ioavr128da28.h rather than assumed:

   1. OSCCFG / SYSCFG0 / SYSCFG1 / CODESIZE / BOOTSIZE:
      (equivalent to fuse write:
      `--fuses 2:0b00000000 5:0b11001001 6:0x04 7:0x00 8:0x00`,
      i.e. offsets OSCCFG=0x00, SYSCFG0=0xC9, SYSCFG1=0x04,
      CODESIZE=0x00, BOOTSIZE=0x00). Decoding 0xC9 = CRCSRC_NOCRC_gc (0xC0)
      | RSTPINCFG_RST_gc (0x08) | FUSE_EESAVE (0x01), and 0x04 = SUT_8MS_gc
      (`eesave=enable,resetpin=reset, startuptime=8`).

   2. WDTCFG / BODCFG are not in that recorded fuse write , so instead they're
      set here explicitly, and their values (0x00 each) match both
      FUSE_WDTCFG_DEFAULT / FUSE_BODCFG_DEFAULT in the header.

   CODESIZE / BOOTSIZE: both 0 — no bootloader partition, single-image
   application uses the full 128KB flash: programming is UPDI-only.

   To inspect:
     avr-objdump -s -j .fuse MiyoCalc40_firmware.elf
*/

FUSES =
{ .WDTCFG   = PERIOD_OFF_gc | WINDOW_OFF_gc,
  .BODCFG   = ACTIVE_DISABLE_gc | SAMPFREQ_128Hz_gc | LVL_BODLEVEL0_gc | SLEEP_DISABLE_gc,
  .OSCCFG   = CLKSEL_OSCHF_gc,
  .SYSCFG0  = CRCSEL_CRC16_gc | CRCSRC_NOCRC_gc | RSTPINCFG_RST_gc | FUSE_EESAVE,
  .SYSCFG1  = SUT_8MS_gc,
  .CODESIZE = 0x00,
  .BOOTSIZE = 0x00,
};
