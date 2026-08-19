#ifndef CCP_H
#define CCP_H

#include <avr/io.h>
#include <stdint.h>

/* CCP-protected register write. Unlocks the register for 4 cycles using the
   IOREG or SPM signature, then writes value before the window closes.

   To set or unset speficic bits in a register (when you cannot disturb other bits in the same register):

   Set bit(s):
   ccp_write_ioreg((void *)&CPUINT.CTRLA,
                   CPUINT.CTRLA | CPUINT_IVSEL_bm);

   Clear bit(s):
   ccp_write_ioreg((void *)&CPUINT.CTRLA,
                   CPUINT.CTRLA & ~CPUINT_IVSEL_bm);

   Read the register in normal code (outside the CCP window), compute the new value, then pass the result to `ccp_write_ioreg()` which does the unlock + store in ≤4 cycles.
   This is safe because the CCP protection only gates the *write* — the read is unrestricted.

   The only caveat: if an ISR could modify the same register between your read and the CCP write, disable interrupts around the pair.
   For `CPUINT.CTRLA` that's typically not a concern (you're usually doing this before `sei()` anyway), but it's worth keeping in mind for registers touched by ISRs.

*/
extern void ccp_write_io(void* addr, uint8_t magic, uint8_t value);

static inline void ccp_write_ioreg(void* addr, uint8_t value)
{ ccp_write_io(addr, CCP_IOREG_gc, value);
}

static inline void ccp_write_spm(void* addr, uint8_t value)
{ ccp_write_io(addr, CCP_SPM_gc, value);
}

#endif /* CCP_H */
