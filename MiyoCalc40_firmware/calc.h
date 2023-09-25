/*
 * Regular (non-programming) calculator functionality (i.e. 'calc mode' stuff)
 */

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef CALC_H
  #define CALC_H
  
  

  // function prototypes
  void do_nothing(uint8_t keycode);
  void enter_number(uint8_t keycode);
  void enter_enter(uint8_t keycode);
  void apply_func_1to1(uint8_t keycode);
  void apply_func_2to1(uint8_t keycode);

#endif

#ifdef __cplusplus
  }
#endif
