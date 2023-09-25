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
const action PROGMEM ACT_0 = {KC_0, &enter_number, {0, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_1 = {KC_1, &enter_number, {1, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_2 = {KC_2, &enter_number, {2, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_3 = {KC_3, &enter_number, {3, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_4 = {KC_4, &enter_number, {4, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_5 = {KC_5, &enter_number, {5, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_6 = {KC_6, &enter_number, {6, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_7 = {KC_7, &enter_number, {7, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_8 = {KC_8, &enter_number, {8, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_9 = {KC_9, &enter_number, {9, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_A = {KC_A, &enter_number, {'A' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_B = {KC_B, &enter_number, {'B' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_C = {KC_C, &enter_number, {'C' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_D = {KC_D, &enter_number, {'D' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_E = {KC_E, &enter_number, {'E' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};
const action PROGMEM ACT_F = {KC_F, &enter_number, {'F' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

const action PROGMEM ACT_ADD = {KC_ADD, &apply_func_2to1, {39, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

const action PROGMEM ACT_ENTER = {KC_ENTER, &enter_enter, {'E' - MCFLETOFFSET, 'N' - MCFLETOFFSET, 'T' - MCFLETOFFSET, 'E' - MCFLETOFFSET, 'R' - MCFLETOFFSET}};

const action PROGMEM ACT_NOP = {KC_NOP, &do_nothing, {'N' - MCFLETOFFSET, 'O' - MCFLETOFFSET, 'P' - MCFLETOFFSET, MCFNULCHAR, MCFNULCHAR}};
#define x____x &ACT_NOP

// define cards
// we should have 3 separate cards objects (calccards, progcards, configcards)
enum calcproglayers
{
  baseLayer = 0,
  fLayer,
  gLayer,
  hLayer
};

// calculator mode cards
// to do: need to add one dimension to this table, as the first dimension, to allow for 'switching modes from algebra to trig to fin, etc'
const action *calc_cards[4][NUM_ROW_PINS][NUM_COLUMN_PINS] = 
{
  [baseLayer] = 
    { {       x____x,    &ACT_ENTER,        x____x,        x____x,        x____x,        x____x,        x____x,        &ACT_7,        &ACT_8,        &ACT_9}, // 10 columns in row 0 (top)
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        &ACT_4,        &ACT_5,        &ACT_6}, // 10 columns in second row (row 1)
      {       x____x,      &ACT_ADD,        x____x,        x____x,        x____x,        x____x,        x____x,        &ACT_1,        &ACT_2,        &ACT_3}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        &ACT_0,        x____x,        x____x}
    },
    
  [fLayer] = 
    { {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}
    },
    
  [gLayer] = 
    { {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}
    },
    
  [hLayer] =
    { {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x},
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}, 
      {       x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x,        x____x}
    },
};

// programming mode cards


// config mode cards


// function definitions

// from keypos_r and keypos_c (global variables), return an action structure, via a lookup of the keypos from the currently active card
action keytoaction()
{
  return *calc_cards[baseLayer][keypos_r][keypos_c]; // lookup action struct pointer in card and dereference it. to-do: need this lookup to be aware of current layer (base, f, g, h) and mode (calc, prog, config)
} // keytoaction()
