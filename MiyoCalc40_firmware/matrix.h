// function prototypes for matrix.c
#include <stdint.h>

/*
#ifdef __cplusplus
  extern "C" {
#endif
*/

#ifndef MATRIX_H
  #define MATRIX_H
  
  #define NUM_COLUMN_PINS 10
  #define NUM_ROW_PINS 4
  
  void setupMatrix();
  void scanKB();
  
  extern uint8_t keypos_r;
  extern uint8_t keypos_c;

#endif

/*
#ifdef __cplusplus
  }
#endif
*/
