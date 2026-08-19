/* ccp.S — CCP-protected register write (AVR-GCC / avr-binutils)
 *
 * void ccp_write_io(void *addr, uint8_t magic, uint8_t value)
 *
 * GCC AVR calling convention (avr25 and up):
 *   r25:r24  addr   (16-bit pointer)
 *   r22      magic  (CCP_IOREG_gc or CCP_SPM_gc)
 *   r20      value
 *
 * The CCP write window is exactly 4 CPU cycles. The sequence below
 * fits: out CCP (1), st Z (2-cycle on AVRDx) — writing must begin
 * within 4 cycles of the CCP unlock.
 */

#include <avr/io.h>

    .section .text.ccp_write_io, "ax", @progbits
    .global  ccp_write_io
    .type    ccp_write_io, @function

ccp_write_io:
#ifdef RAMPZ
    out     _SFR_IO_ADDR(RAMPZ), r1    /* clear bits 23:16 of Z (r1 = 0 by ABI) */
#endif
    movw    r30, r24                   /* Z = addr */
    out     _SFR_IO_ADDR(CCP), r22    /* open CCP window */
    st      Z, r20                     /* write value — must land within 4 cycles */
    ret

    .size ccp_write_io, .-ccp_write_io
