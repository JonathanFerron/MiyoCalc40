// function prototypes for matrix.c
#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef MATRIX_H
  #define MATRIX_H
  
  void setupMatrix();
  void scanKB();
  
  extern uint8_t keypos_r;
  extern uint8_t keypos_c;

#endif

#ifdef __cplusplus
  }
#endif
