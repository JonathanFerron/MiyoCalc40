#ifndef GPIO_H
#define GPIO_H

#include <avr/io.h>
#include <stdint.h>

/* Lightweight GPIO pin descriptor -- encodes a port + bitmask pair so
   callers (lcd.h, matrix.cpp) can pass "a pin" around the way the Arduino
   pin-number scheme they replace did, without an Arduino core underneath.
   Mirrors KeyDU-Citrouille90/firmware/KeyDU.App/gpio.h, trimmed to only
   what this project currently uses (no pullup/invert/ISC helpers yet --
   matrix.cpp still configures those in bulk via direct PINCTRLSET writes). */
typedef struct
{ uintptr_t port_addr;
  uint8_t   mask;
} gpio_pin_t;

#define GPIO_PIN(port, pin_n) \
  { .port_addr = (uintptr_t)&(port), .mask = (1u << (pin_n)) }

/* Internal accessor -- not for external use */
#define _PORT(p)  ((PORT_t *)(p).port_addr)

#define GPIO_SET_OUTPUT(p)  (_PORT(p)->DIRSET = (p).mask)
#define GPIO_HIGH(p)        (_PORT(p)->OUTSET = (p).mask)
#define GPIO_LOW(p)         (_PORT(p)->OUTCLR = (p).mask)

/* Non-zero (not necessarily 1) if pin is high, zero if low. */
#define GPIO_READ(p)        (_PORT(p)->IN & (p).mask)

#endif /* GPIO_H */
