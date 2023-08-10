/* Matrix scanning examples:

1. Open RPN Calc: slower but more robust
define number_of_columns and number_of_rows
declare const for row pin and column pin arrays (contain pin numbers)

helper function (scan keyboard), returns an uint16_t, takes no argument
  pressed_row = 0
  pressed_column = -1
  set all output column pins to high
  scan columns: for each column
  	set one column pin output low
  	small 10 microsecond delay for transitional processes to finish
  	read row pins: for each row
  		read row pin status
  		if row pin is low:
  			pressed_row = current row
  			pressed_column = current column
  	end loop over all rows

  	set column pin back to high

  end loop over all columns

  reset all output column pins to low

  return pressed_column + pressed_row * number_of_columns + 1

2. an3407: faster but less robust
define number_of_columns and number_of_rows
declare const for row pin and column pin arrays (contain pin numbers)

set column as input pull-up (already done in main loop)
Rows as output low (already done in main loop)
Read each column to find which has been pulled low
This will tell us which column was pressed

set row pins as input pull-up
Set entire column axis as output low
Read voltage on each rows to find which has been pulled low
This will tell us which row was pressed

reset columns pins to input pull-up and row pins as output low

Combine row and column into an int (e.g. 2 nibbles encoded in an uint8) or a set of 2 ints (one for each value, could be in a bitfield) and return the result

*/

// Includes



// Defines



// Global variables



// helper matrix scanning functions code

#include "matrix.h"