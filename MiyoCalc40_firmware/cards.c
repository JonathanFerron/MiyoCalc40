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
  lookup from calccards, progcards, configcards matrices an 'action' structure: could each be n x 4 x 10 matrices (n being 4 for calc mode and prog mode for unshifted, then f, g, 
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
#define x_____x KC_NOP

// structures
// https://aticleworld.com/function-pointer-in-c-struct/
// https://stackoverflow.com/questions/1350376/function-pointer-as-a-member-of-a-c-struct
// https://www.codeproject.com/Tips/800474/Function-Pointer-in-C-Struct

typedef struct _action
{
  uint8_t keycode;  // e.g. KC_ADD
  void (*fct)(uint8_t keycode_arg);  // function pointer to the actual function to execute (if any), e.g. add()
  uint8_t mnemonic[5]; // 1 to 5 indexes into the font table (0 to 47). Use MCFNULCHAR for empty characters.
  // may want to add an 'opmode' variable here (define possible values in an enum) to indicate the type of action (actions that modify x only, actions that do something with x and y and drop one element from stack, etc.)
} action;


// global variables

// each action object to be a const instance of the action struct
const action ACT_0 = {KC_0, &enter_number, {0, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR, MCFNULCHAR}};

// could define cards right here
// this is how it's done by QMK: Would go with uint8_t instead, and we would have 3 separate cards objects (calccards, progcards, configcards)
// could use the x_____x trick for KC_NOP as well if it helps.
enum calcproglayers
{
  baseLayer = 0,
  fLayer,
  gLayer,
  hLayer
};

const uint8_t PROGMEM calc_cards[4][NUM_ROW_PINS][NUM_COLUMN_PINS] = 
{
  [baseLayer] = 
    { {}, // 10 columns in first row (from top)
      {}, // 10 columns in second row (from top)
      {}, 
      {}
    },
  [fLayer] = 
    { {}, 
      {}, 
      {}, 
      {}
    },
  [gLayer] = 
    { {}, 
      {}, 
      {}, 
      {}
    },
  [hLayer] =
    { {}, 
      {}, 
      {}, 
      {}
    }
};

// function definitions


