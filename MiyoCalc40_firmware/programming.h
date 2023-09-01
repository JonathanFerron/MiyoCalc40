#ifndef PROGRAMMING_H
#define PROGRAMMING_H


#endif

/* Programming features ('program mode' calculator functionalies):
 * 
 * IF
 *   X?0 (6 keycodes) or X?Y (6 keycodes) or Flag(n) = True (1 keycode, provide flag number on next instruction)
 *     do ABC
 * ELSE
 *     do def
 * ENDIF
 * 
 * 3 keycodes, one for IF, one for ELSE, one for ENDIF
 * 
 * SF(n): SF keycode, to set flag to true or 1, with 'n' provided as 'next instruction' (on the next row)
 * n between 0 and 7 in a 8 bit flag register (uint_8 or byte) 
 * CF(n): CF keycode, to set flag to false or 0, with 'n' provided as 'next instruction' (on the next row)
 * 
 * 
 * CASE
 *   var to test (0 (by convention) for X, 1 for Y, ... ?)
 *   WHEN
 *     val 1
 *       do abc
 *   WHEN
 *     val 2
 *       do def
 *   OTHER
 *       do ghi
 * ENDCAS
 * 
 * 4 keycodes: WHEN, CASE, OTHER, ENDCAS
 * 
 * FOR
 *   init count (16 bit signed integer)
 *   COUNT? (reuse the 'X?Y' keycode and repurpose as a COUNT?n in the context of a FOR loop)
 *   n (float from which we will take integer portion)
 *   Do stuff
 * NEXT
 *   increment / decrement to count here (float from which we will take integer portion)
 * 
 * 2 keycodes: FOR and NEXT
 * 
 * WHILE
 *   X?0 or X?Y or Flag(n) = True
 *   do stuff
 * WEND
 * 
 * 2 keycodes: WHILE and WEND
 * 
 * DO
 *   do stuff
 * LOOPWH
 *   X?0 or X?Y or Flag(n) = True
 * 
 * 2 keycodes: DO and LOOPWH
 * 
 * May need around 30 to 36 keycodes in total for programming feature.
 * 
 * May want to also implement an EXIT instruction to force loops and IF to return and a CYCLE instruction to force WHILE and DO loops to loop
 * 
 * Control structures and logic test on h shift layer (type 'f' 3 times to get to 'h' layer)
 * 
 * Function pointers in prog cards e.g. record_keycode(keycode) (will insert as well), move_cursor_up(NOP), move_cursor_down(NOP), delete_keycode(NOP)
 * NOP keycode to be 0xFF
 * 
 * XEQ, Step, PGM, Solve: from calc mode works much same way as STO and RCL (wait for pgm ident)
 * 
 * Programs: array of 36 'dynamic array' structs (one for each of 36 programs, 0 to 9 and A to Z):
 * each struct could look like this:
 *  uint16 size, default to 0
 *  uint16 capacity, default to 0
 *  uint8 *prog, default to null pointer
 *  char name, name of program ('0', '1', ..., 'A', ..., 'Z')
 *  in a given program, allocate 16 elements at a time and dynamically grow size (number of program rows): so allocate 16 in capacity and *prog at once, but only 
 *  increment size and fill in *prog one row at a time  
 * 
up and down: avail as direct keys when in prog mode (don't exist in calc mode); put them on keys that are unlikely to be recorded in prog mode, eg factorial and clr reg 
(top most center red keys), and shift these two functions in prog mode

avail as shifted key to go in (expect prog label next) and out of prgm mode: prgm (pr)

maximum 36 pgm labels

as shifted key in prg mode only:
clr prgm (cp)

shftd keys from calc Mode:
xeq (run or go) and step (execute step by step), which req prgm label
solv: find input x that makes end result be 0 after application of pgrm instructions

Add ability to XEQ program step by step: with no intervention in the middle aside from clicking 'next': could go on g(XEQ) = f(f(factorial)) from calc mode

pgm saved in sram by Default. can be backup in flash (appdata) using 'save' or 'push' function (must be done prior to removng battery to keep pgms)

load, save (entire set of all programs), pull (a single prgm) as shifted keys from pgm mode
wake, sleep button: use interrupt approach to wake mcu

use 1 or 2 512 byte pages for programs. 1 byte keycode per instruction (maximum of 256 keycodes)
 * 
 */
