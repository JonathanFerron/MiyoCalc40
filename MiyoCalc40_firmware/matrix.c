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
#include "matrix.h"
#include "Arduino.h"

// Defines
//define number_of_columns and number_of_rows

// Global variables
//declare const for row pin and column pin arrays (contain pin numbers)
const uint8_t row_pin_array[NUM_ROW_PINS] = {
		PIN_PD0, PIN_PD1, PIN_PD2, PIN_PD3};

const uint8_t column_pin_array[NUM_COLUMN_PINS] = {
		PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3, PIN_PD4, PIN_PD5, PIN_PD6, PIN_PD7, PIN_PF1, PIN_PF0};
    
const uint8_t unused_pin_array[2] = {PIN_PA5, PIN_PA0};

uint8_t keypos_r;
uint8_t keypos_c;
/*
struct keyposS
{
  unsigned int r : 4;
  unsigned int c : 4;
} keypos;
*/

// helper matrix scanning functions code

void setupMatrix()
{
  // set-up pull-up on column and row pins
  PORTA.PINCONFIG = PORT_PULLUPEN_bm;  
  PORTC.PINCTRLSET = 0b00001111;
  PORTD.PINCTRLSET = 0b11111111;
  PORTF.PINCTRLSET = 0b00000011;
  
  // set-up column pins as input 
  //PORTC.DIR &= ~(PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm); 
  //PORTD.DIR &= ~(PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm); 
  //PORTF.DIR &= ~(PIN0_bm, PIN1_bm); 
  PORTC.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
  PORTD.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
  PORTF.DIRCLR = PIN0_bm | PIN1_bm;
  
  // set row pins as output low
  //PORTD.DIR |= PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm; // output
  //PORTD.OUT &= ~(PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm); // low
  PORTD.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
  PORTD.OUTCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;

  // Disable the digital input buffer on the unused pins (porta.pin0ctrl = port_pullupen_bm | port_isc_input_disable_gc)
  PORTA.PINCONFIG = PORT_PULLUPEN_bm | PORT_ISC_INPUT_DISABLE_gc;
  PORTA.PINCTRLSET = 0b00100001;
  
  keypos_r = 0xff;  // initialize keypos to 'null', which we'll define as 0xff by convention
  keypos_c = 0xff;
}

/*
Read each column to find which has been pulled low
This will tell us which column was pressed

Set column pins as output low
set row pins as input

Read each row to find which has been pulled low
This will tell us which row was pressed

reset columns pins to input and row pins as output low

Combine row and column into a keypos struct and return the result (global var keypos)
*/
void scanKB()
{
  // read each column to find if one has been pulled low
  // TODO: if necessary, look into optimizing this loop: could use digitalReadFast() if we were to unroll the loop to use constant pin numbers. 
  // It does make the code harder to modify after, however.
  for (uint8_t col = 0; col < NUM_COLUMN_PINS; col++)  
  {
    if ( digitalRead(column_pin_array[col]) == LOW )  
    {
      // we now know that 'c' is the column of the button pressed      
      
      // set column pins as output low
      PORTC.DIRTGL = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;  // toggle direction to output
      PORTD.DIRTGL = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
      PORTF.DIRTGL = PIN0_bm | PIN1_bm;
      
      PORTC.OUTCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;  // low: can this be optimized out by running it just once in setupMatrix() ?
      PORTD.OUTCLR = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
      PORTF.OUTCLR = PIN0_bm | PIN1_bm;
      
      // set row pins as input
      PORTD.DIRTGL = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
      
      // Read each row to find which has been pulled low
      for (uint8_t row = 0; row < NUM_ROW_PINS; row++)
      {
        if ( digitalRead(row_pin_array[row]) == LOW )
        {
          // we now know that 'r' is the row of the button pressed
          // Combine row and column into a keypos struct and return the result (global var keypos)         
          keypos_c = col;  // there is a cast from 8 to 4 bit here
          keypos_r = row;
          break;  // break out of the loop across each row
        }  // if a row pin was low        
        
      } // loop across each row
      
      // reset columns pins to input and row pins as output low
      // column pins
      PORTC.DIRTGL = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;  // toggle direction to input
      PORTD.DIRTGL = PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
      PORTF.DIRTGL = PIN0_bm | PIN1_bm;
      
      // row pins
      PORTD.DIRTGL = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm; // toggle direction to output
      PORTD.OUTCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm; // low: can this be optimized out as well?
      
      break;  // break out of the loop across each column
    } // if a column pin was low
    
  } // loop across each column
  
} // scanKB()

