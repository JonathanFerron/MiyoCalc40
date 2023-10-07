/*
  cards.c

  allow flexible config via cards
 
  card = algebra, trig, fin, sci, eng, prob, computer science, woodworking, valuation, ...

  for each card, can have a base layer, and multiple shift layers (f, g, h)

  set of all possible keycodes: could be a bunch of defines e.g. #define KC_ADD 0xBB, store them in cards.h or keycodes.h. Use 0x00 as NOP.

  keymap translation function: keytoaction(keypos)
  keypos: row and column number of key that was pressed, see matrix.c

  global variables used:
  mode: calc, prog, config
    include ability to swap 'card' from config screen
  configscreen: ... list out the various screens (only relevant in config mode)
  f, g, h shift registers (true / false): for g, hit f twice; for h, hit h three times

  do this:
  lookup from calccards, progcards, configcards matrices an 'action' structure: 
    each 'card' could each be n x 4 x 10 matrices (n being 4 for calc mode and prog mode for unshifted, then f, g, 
    and h, and n being the number of 'screens' that are presented in the config mode)

  action structure: contains a keycode (for programming mode, if it's an action that's programmable, otherwise NOP=0xFF), 
    a function pointer to the implementation of the actual action (function to always take
    one single argument, the keycode, though it may at time not be used, argument to be 
    looked up and then 'provided' to the function called via the pointer), and a 'mnemonic' for the programming mode 
    (uint8 matrix containing indexes of different characters from the 'font' table)

  return: the action structure

  keycode: uint8, max of 256 keycodes 
  function mnemonic: 1 to 5 char (for programming) e.g. "+" or "7" or "Enter"
    
*/

/* 
How this is done in OpenRPNCalc:
Using the calc_on_key function
  switch case statement based on matrixkeypos code: this calls 
  the 'enter_key', change_trigmode (deg, rad), enter_shift (toggles f, g, h shift flags), change_dispmode (norm, sci, eng) or 
  change_precision (nbr of decimals to display) function to trigger the execution of the action

enter_key calls apply_op with actual action to execute: in MiyoCalc we would just return
  a pointer to the function to execute and rest is done in the 'calc.c', 'programming.c' and 'config.c' files.

*/

// includes
#include <stdint.h>
#include "cards.h"
#include "fonts.h"
#include "matrix.h"
#include "calc.h"

// defines


// structures
// https://aticleworld.com/function-pointer-in-c-struct/
// https://stackoverflow.com/questions/1350376/function-pointer-as-a-member-of-a-c-struct
// https://www.codeproject.com/Tips/800474/Function-Pointer-in-C-Struct

// global variables
// each action object to be a const instance of the action struct
const action ACT_0 = {KC_0, &enter_number, {0, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_1 = {KC_1, &enter_number, {1, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_2 = {KC_2, &enter_number, {2, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_3 = {KC_3, &enter_number, {3, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_4 = {KC_4, &enter_number, {4, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_5 = {KC_5, &enter_number, {5, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_6 = {KC_6, &enter_number, {6, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_7 = {KC_7, &enter_number, {7, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_8 = {KC_8, &enter_number, {8, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_9 = {KC_9, &enter_number, {9, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_A = {KC_A, &enter_number, {'A' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_B = {KC_B, &enter_number, {'B' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_C = {KC_C, &enter_number, {'C' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_D = {KC_D, &enter_number, {'D' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_E = {KC_E, &enter_number, {'E' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_F = {KC_F, &enter_number, {'F' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

const action ACT_POINT = {KC_POINT, &enter_decpoint, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};  // need to implement a decimal point character in the main font table
const action ACT_CHGSGN = {KC_CHGSGN, &enter_sign, {39, 36, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_EEX = {KC_EEX, &enter_exp, {'E' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_PI = {KC_PI, &apply_const, {'P' - MCFLETOFFSET, 'I' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

const action ACT_ADD = {KC_ADD, &apply_func_2to1, {39, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_SUBS = {KC_SUBS, &apply_func_2to1, {36, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_MULT = {KC_MULT, &apply_func_2to1, {'X' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};  // need to implement a multiplication character in the main font table
const action ACT_DIV = {KC_DIV, &apply_func_2to1, {45, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_POW = {KC_POW, &apply_func_2to1, {'P' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'W' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_ROOT = {KC_ROOT, &apply_func_2to1, {'R' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'T' - MCFLETOFFSET, MCFNULCHAR}};
const action ACT_MULTINV = {KC_MULTINV, &apply_func_2to1, {1, 45, 'X' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_ABS = {KC_ABS, &apply_func_1to1, {'A' - MCFLETOFFSET, 'B' - MCFLETOFFSET, 'S' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_SQRT = {KC_SQRT, &apply_func_1to1, {'S' - MCFLETOFFSET, 'Q' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'T' - MCFLETOFFSET, MCFNULCHAR}};
const action ACT_SQUARE = {KC_SQUARE, &apply_func_1to1, {'X' - MCFLETOFFSET, 42, 2, MCFNULCHAR, MCFNULCHAR}};
//KC_LN
//KC_EXP
//KC_FACT
//KC_ROUND
//KC_FRAC
//KC_INTEG

const action ACT_ENTER = {KC_ENTER, &enter_enter, {'E' - MCFLETOFFSET, 'N' - MCFLETOFFSET, 'T' - MCFLETOFFSET, 'E' - MCFLETOFFSET, 'R' - MCFLETOFFSET}};
const action ACT_DROPX = {KC_DROPX, &enter_drop, {'D' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'P' - MCFLETOFFSET, 'X' - MCFLETOFFSET}};
const action ACT_SWAP = {KC_SWAP, &enter_swap_xy, {'X' - MCFLETOFFSET, 37, 36, 38, 'Y' - MCFLETOFFSET}};
const action ACT_ROLLDN = {KC_ROLLDN, &enter_rotate, {'R' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'D' - MCFLETOFFSET, 'N' - MCFLETOFFSET}};
const action ACT_ROLLUP = {KC_ROLLUP, &enter_rotate, {'R' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'U' - MCFLETOFFSET, 'P' - MCFLETOFFSET}};
const action ACT_BKSCLRX = {KC_BKSCLRX, &enter_backspace_clrx, {'C' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'X' - MCFLETOFFSET, MCFNULCHAR}};
const action ACT_LASTX = {KC_LASTX, &enter_lastx, {'L' - MCFLETOFFSET, 'A' - MCFLETOFFSET, 'S' - MCFLETOFFSET, 'T' - MCFLETOFFSET, 'X' - MCFLETOFFSET}};
const action ACT_CLRSTCK = {KC_CLRSTCK, &enter_clear, {'C' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'S' - MCFLETOFFSET, 'T' - MCFLETOFFSET, 'K' - MCFLETOFFSET}};

const action ACT_STOREG = {KC_STOREG, &toggle_mem_mode, {'S' - MCFLETOFFSET, 'T' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'G' - MCFLETOFFSET}};
const action ACT_RCLREG = {KC_RCLREG, &toggle_mem_mode, {'R' - MCFLETOFFSET, 'C' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'G' - MCFLETOFFSET}};
const action ACT_CLRREG = {KC_CLRREG, &toggle_mem_mode, {'C' - MCFLETOFFSET, 'L' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'R' - MCFLETOFFSET, 'G' - MCFLETOFFSET}};

const action ACT_SIN = {KC_SIN, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_COS = {KC_COS, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_TAN = {KC_TAN, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_ASIN = {KC_ASIN, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_ACOS = {KC_ACOS, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
const action ACT_ATAN = {KC_ATAN, &apply_func_1to1, {'S' - MCFLETOFFSET, 'I' - MCFLETOFFSET, 'N' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};

//KC_BWAND
//KC_BWOR
//KC_BWXOR	
//KC_BWNOT	  

//KC_MODEBIN	
//KC_MODEOCT	
//KC_MODEDEC	 
//KC_MODEHEX	 

//KC_MODERAD	
//KC_MODEDEG	

//KC_DATE	  
//KC_DATEDIFF	

const action ACT_NOP = {KC_NOP, &do_nothing, {'N' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'P' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};

const action ACT_SHFT_B = {KC_NOP, &enter_shift_base, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_SHFT_F = {KC_NOP, &enter_shift_f, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_SHFT_G = {KC_NOP, &enter_shift_g, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action ACT_SHFT_H = {KC_NOP, &enter_shift_h, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

const action ACT_PWR_DOWN = {KC_NOP, &power_down, {MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

#define x____x &ACT_NOP

// define cards
// we should have 3 separate cards objects (calccards, progcards, configcards)


// calculator mode cards
// to do: need to add one dimension to this table, as the first dimension, to allow for 'switching modes from algebra to trig to fin, etc'
const action *calc_cards[4][NUM_ROW_PINS][NUM_COLUMN_PINS] = 
{
  [baseLayer] = 
    { {    &ACT_SWAP,    &ACT_ENTER,    &ACT_DROPX,   &ACT_ROLLDN,        x____x,        x____x,        x____x,        &ACT_7,        &ACT_8,        &ACT_9}, // 10 columns in row 0 (top)
      { &ACT_BKSCLRX,     &ACT_MULT,      &ACT_DIV,        x____x,        x____x,        x____x,        x____x,        &ACT_4,        &ACT_5,        &ACT_6}, // 10 columns in second row (row 1)
      {       x____x,      &ACT_ADD,     &ACT_SUBS,        x____x,        x____x,        x____x,   &ACT_CHGSGN,        &ACT_1,        &ACT_2,        &ACT_3}, 
      {       x____x,  &ACT_MULTINV,     &ACT_SQRT,        x____x,        x____x,        x____x,        x____x,        &ACT_0,    &ACT_POINT,   &ACT_SHFT_F}
    },
    
  [fLayer] = 
    { {       x____x,        x____x,        x____x,   &ACT_ROLLUP,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x, &ACT_PWR_DOWN,        x____x,        x____x,        x____x,        x____x,   &ACT_SHFT_G}
    },
    
  [gLayer] = 
    { {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,   &ACT_SHFT_H}
    },
    
  [hLayer] =
    { {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,   &ACT_SHFT_B}
    },
};

// programming mode cards


// config mode cards


// function definitions

// from keypos_r and keypos_c (global variables), return an action structure, via a lookup of the keypos from the currently active card
action keytoaction()
{
  // if in calc mode (need to implement an 'if' or switch / case statement here)
  // calc vs prog vs config mode could be an enum, variable name could be 'activemode'
  return *calc_cards[shift][keypos_r][keypos_c]; // lookup action struct pointer in card and dereference it. will eventually add a first dimension for the active 'card', variable name could be 'activecard'

  // need to implement another lookup when in prog mode here
  
  // and a final lookup when in config mode
  
} // keytoaction()
