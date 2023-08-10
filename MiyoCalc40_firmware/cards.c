/*
  cards.c

  likely want the set of possible keycodes to be a bunch of defines e.g. #define KC_ADD 0xBB, perhaps store them in cards.h or keycodes.h

  keymap translation function: keytoaction(matrixkeypos)
  matrixkeypos: bitfield containing row and column number of key that was pressed

  global variables used:
  mode: calc, prog, config
  configscreen: ... list out the various screens (only relevant in config mode)
  f, g, h shift registers (true / false)

  do this:
  lookup from calccards, progcards, configcards matrices an 'action' structure: could each be 40 x n matrices (n being 4 for calc mode and prog mode for unshifted, then f, g, 
  and h, and n being the number of 'screens' that are presented in the config mode)

  action structure: contains a keycode (for programming mode, if it's an action that's programmable, otherwise 0x00), 
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