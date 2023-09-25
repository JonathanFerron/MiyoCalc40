/*
start by copying a lot of code from OpenRPNCalc, go through everything, simplify
and modify as necessary in the context of the 'cards', actions', 'keycodes',
and variable size (max 8) stack that grows and shrinks.

openrpncalc pseudocode and structure (from calc.c):
  setup_calc():
    trigmode = DEG   we'll use an enum instead
    dispmode = NORM   we'll use an enum instead. This is for the number display mode: NORM, SCI, ENG
    shift = noshift      we have three possible shifts (four with 'no shift'), so use an enum instead
    precision = 10
    call set_trigconv
    call clear_input
    call clear_stack
    call draw_status (we'll implement the 'draw' functions in lcd.cpp)
    call draw stack

  apply_op(): is called to trigger the execution of the actions
    this calls the apply_enter, apply_stack, apply_const, apply_memory_rcl,
    apply_memory_sto, convert_input, clear_input, apply_func_1to1,
    apply_func_2to1, applyfunc2to2, draw_status and
    draw_stack functions (anything that starts with 'draw' should probably go
    in our lcd.cpp). code depends on the use of the 'op mode' portion of the
    16 bit keycode: we could have a lookup table to provide this 'op mode'
    based on our 8 bit keycode, if necessary, or more probably we could just
    go and use the 'enter_number(KC_ENTER_1), enter_enter(KC_ENTER), apply_func_1to1(KC_SIN),
    apply_func_2to1(KC_PLUS) etc. functions as our function pointers' that way the
    functions from OpenRPNCalc can be used almost as-is (use 8 bit keycodes instead
    of 16 bits, and use KC_SIN instead of OP_SIN). need to insert the uses of
    the convert_input ad clear_input from the apply_op function inside the 
    apply_func_atob() functions.

  t_input input : structure that we likely want to copy
  stack: array of double (use single to start with)
  lastx: double (use single)
  error_flag: boolean
  variables: array of double (use single to start with), storage space for variables, perhaps change the name to memoryregisters


use variable depth stack (min 1, max 8, elements): first two still refered to as x and y, but really they are s0 and s1
initialize with only 1 variable in the stack (x), at 0.0

show 6 stack registers on screen (3 high, reduce nbr of dec for 6), include mem (heap) reg up to a grand total of 6: X, Y, Z, T, S, R, (Q, P)

Display 6 max items from the stack and memory registers on screen (with 6 digits when displaying 6 items). When displaying only 3 stack items, show up to 9 digits.

Show deg (little degree sign, circle) vs rad (little pi sign) mode indicatod in corner.

show shift indicator in corner of screen: f g h (small font)

memory registers (heap): maximum of 36 registers from 0 to 9 and from A to Z. we can refer to them as R(0), R(1), etc. 

*/

// Includes
#include <math.h>

#include "calc.h"
#include "cards.h"

// defines


// global variables
double stack[4];            // Stack values {X, Y, Z, T}
double lastx;               // Last X value
int error_flag;             // Error flag: 1 if error occurred as a result of last operation

double variables[256];      // Storage space for variables

int trigmode;     // Trigonometric mode: 0-DEG, 1-RAD
int dispmode;     // Display mode: 0-NORM, 1-SCI, 2-ENG
double trigconv;  // Trigonometric conversion constant, 1 for RAD, (pi/180.) for DEG
int shift;
int precision;         // Precision in NORMAL context


// Table of maximal mantissa values for different precisions
uint64_t max_mantissa[10] = {
		9LL,
		99LL,
		999LL,
		9999LL,
		99999LL,
		999999LL,
		9999999LL,
		99999999LL,
		999999999LL,
		9999999999LL
};

// Input structure typedef
typedef struct {
  char mantissa[12];   // Mantissa digits array
  char sign;           // Sign: 0 for "+", 1 for "-"
  char exponent[3];    // Exponent digits array
  char expsign;        // Sign of the exponent: 0 for "+", 1 for "-"
  char started;        // 1 if input mode is active
  char enter_pressed;  // 1 if "ENTER" key was just pressed
                       // (such that we need to replace the current X value by the new number)
  char expentry;       // 1 if exponent is being entered
  int8_t point;        // Decimal point position
  int8_t mpos;         // Number of mantissa digits entered
} t_input;

t_input input; // Input structure

// Set the trigconv constant depending on current trigonometric mode
void set_trigconv() 
{
  if (trigmode == 0) 
  {
    trigconv = M_PI/180.; 
  }
  if (trigmode == 1) 
  {
    trigconv = 1; 
  }
}

// Clear stack and error flag
void clear_stack() {
	stack[0] = 0.;
	stack[1] = 0.;
	stack[2] = 0.;
	stack[3] = 0.;
	lastx = 0.;
	error_flag = 0;
}

// Clear input structure
void clear_input() {
	input.mpos = 0;
	input.sign = 0;
	input.point = 0;
	input.started = 0;
	input.enter_pressed = 0;
	input.expentry = 0;
	input.exponent[0] = 0;
	input.exponent[1] = 0;
	input.exponent[2] = 0;
	input.expsign = 0;
}

// intitialization
void calc_init() {
  trigmode = 0;
  dispmode = 0; 
  shift = 0;
  precision = 10;

  set_trigconv(); 

  clear_input();
  clear_stack();
  //draw_status(); // needs to be implemented
  //draw_stack();  // needs to be implemented
}




// Push the number to stack
void stack_push(double num) {
	stack[3] = stack[2];
	stack[2] = stack[1];
	stack[1] = stack[0];
	stack[0] = num;
}

// Drop the value from stack (T register is copied, X is lost)
void stack_drop() {
	stack[0] = stack[1];
	stack[1] = stack[2];
	stack[2] = stack[3];
}

// Rotate stack "upwards" (X->Y, Y->Z, Z->T, T->X)
void stack_rotate_up() {
	double tmp = stack[0];

	stack[0] = stack[3];
	stack[3] = stack[2];
	stack[2] = stack[1];
	stack[1] = tmp;
}

// Rotate stack "downwards" (Y->X, Z->Y, T->Z, X->T)
void stack_rotate_down() {
	double tmp = stack[0];

	stack[0] = stack[1];
	stack[1] = stack[2];
	stack[2] = stack[3];
	stack[3] = tmp;
}

// see draw_char(), draw_number_split(), draw_number_sci(), draw_number_eng(), draw_number_all(), draw_number(), draw_stack(), draw_input() and draw_decpoint() functions from OpenRPNCalc for some ideas on updating LCD with stack

// see draw_status() function from OpenRPNCalc for ideas on updating LCD with shift (f, g, h) and trig mode (degrees vs radian) indicators

// see draw_error() as well from OpenRPNCalc when an error is encountered that should be shown on screen



// function definitions
void enter_number(uint8_t keycode)
{
  if (!input.started)
  {
    if (!input.enter_pressed && !error_flag) 
    {
      stack_push(0); 
      //draw_stack();  // need to implement this
    }
    error_flag = 0; 
    input.started = 1;
    input.enter_pressed = 0;
  }
  if (input.expentry == 0) 
  {
    if (input.mpos < 10 && !(keycode==0 && input.point == 0 && input.mpos == 0)) 
    {
      input.mantissa[input.mpos++] = keycode; 
    }
  } 
  else 
  {
    input.exponent[2] = input.exponent[1];
    input.exponent[1] = input.exponent[0];
    input.exponent[0] = keycode;
  }
  //draw_input();   // need to implement this
} // enter_number()

double convert_input() 
{
  int i; 
  double number = 0.; 
  double shift = 1;

  for (i=0; i<input.mpos; i++) 
  {
    number += input.mantissa[input.mpos-i-1]*shift;
    shift *= 10;
  }
  int exponent = 100*input.exponent[2] + 10*input.exponent[1] + input.exponent[0]; 
  if (input.expsign) exponent = -exponent; 
  if (input.point) exponent -= (input.mpos-input.point); 

  number *= pow(10, exponent); 
  if (input.sign) number = -number; 

  if (!isfinite(number)) 
  {
	  //draw_error(0, 1); // need to implement
	  error_flag = 1;
  }
  return(number); 
}

void enter_enter(uint8_t keycode)
{
  if (input.started) 
  {
    stack[0] = convert_input();    
    clear_input();
  }
  if (error_flag) return; 
  input.enter_pressed = 1;
  stack_push(stack[0]);
  //draw_stack();   // need to implement
} // enter_enter()

void apply_func_1to1(uint8_t keycode)
{
  if (input.started) 
  {
	  stack[0] = convert_input();
    clear_input();
  }
  if (error_flag) return;
  input.enter_pressed = 0;
  
	double f = 0;
  double x = stack[0];
  switch(keycode) 
  {
    case KC_SIN: f = sin(trigconv*x); break;
    /* 
    case OP_COS: f = cos(trigconv*x); break;
    case OP_TAN: f = tan(trigconv*x); break;
    case OP_ASIN: f = asin(x)/trigconv; break;
    case OP_ACOS: f = acos(x)/trigconv; break;
    case OP_ATAN: f = atan(x)/trigconv; break;
    case OP_LG: f = log10(x); break;
    case OP_LN: f = log(x); break;
    case OP_EXP: f = exp(x); break;
    case OP_POW10: f = pow(10., x); break;
    case OP_SQRT: f = sqrt(x); break;
    case OP_SQR: f = x*x; break;
    case OP_INV: f = 1./x; break;
    */
    default: break;
	}
  lastx = stack[0];  
  stack[0] = f;  
  //draw_stack();  // need to implement
} // apply_func_1to1

void apply_func_2to1(uint8_t keycode)
{
  if (input.started) 
  {
	  stack[0] = convert_input();
    clear_input();
  }
  if (error_flag) return;
  input.enter_pressed = 0;
  
  double f = 0;
  double x = stack[0];
  double y = stack[1];

  switch(keycode) {
    case KC_ADD: f = x+y; break;
    /*
    case OP_MINUS: f = y-x; break;
    case OP_MULT: f = y*x; break;
    case OP_DIV: f = y/x; break;
    case OP_POW: f = pow(y, x); break;
    case OP_ROOTX: f = pow(y, 1./x); break;
    case OP_CYX: f = exp(lgamma_r(y+1, &gamma_sign) - lgamma_r(x+1, &gamma_sign) - lgamma_r(y-x+1, &gamma_sign)); break;
    case OP_PYX: f = exp(lgamma_r(y+1, &gamma_sign) - lgamma_r(y-x+1, &gamma_sign)); break;
    case OP_SIGNIF_XY: if (context != CONTEXT_UNCERT) return;
    case OP_POISSON :  f = x < 0 ? NAN : exp(x*log(y) - y - lgamma_r(x+1, &gamma_sign)); break;
    case OP_CHI2_PROB :  f = chisquared_cdf_c(x, y); break;
    */
    default: break;
	}
  lastx = stack[0];  
  stack_drop();
  stack[0] = f;  
  //draw_stack();  // need to implement
} // apply_func_2to1

void do_nothing(uint8_t keycode)
{}
