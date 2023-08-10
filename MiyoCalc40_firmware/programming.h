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
 *   COUNT? (6 keycodes, unless we want to reuse the 'X?Y' keycode and repurpose as a COUNT? in the context of a FOR loop)
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
 */
