/*
 * Based in part on code from OpenRPNCalc by Anton Poluektov: https://github.com/apoluekt/OpenRPNCalc
 * 

Use variable depth stack (min 1, max 8, elements): first two still refered to as x and y, but really they are s0 and s1.
When in variable depthe stack, initialize with only 1 variable in the stack (x), at 0.0

Show up to 6 stack registers on screen (3 high, reduce nbr of digits to 6 for 6), include mem (heap) reg up to a grand total of 6: X, Y, Z, T, S, R, (Q, P).
When displaying only 3 stack items, show up to 9 digits.

*/

// Includes
#include <math.h>

#include "calc.h"
#include "cards.h"
#include "main.h"
#include "fonts.h"
#include "matrix.h"

// defines


// global variables
double stack[4];            // Stack values {X, Y, Z, T}
double lastx;               // Last X value
int error_flag;             // Error flag: 1 if error occurred as a result of last operation

double variables[4][10] = 
{ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};      // Storage space for variables

int trigmode;     // Trigonometric mode: 0-DEG, 1-RAD
int dispmode;     // Display mode: 0-NORM, 1-SCI, 2-ENG
double trigconv;  // Trigonometric conversion constant, 1 for RAD, (pi/180.) for DEG
int precision;    // Precision (number of significant digits)

bool mem_recall_mode;
bool mem_store_mode;
bool mem_clear_mode;
  
int shift;
int current_calc_prog_config_mode;

// Table of maximal mantissa values for different precisions, look into changing this to an int64_t instead
uint64_t max_mantissa[9] = {
		9LL,  // LL stands for long long (64 bit)
		99LL,
		999LL,
		9999LL,
		99999LL,
		999999LL,
		9999999LL,
		99999999LL,
		999999999LL
};


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
  trigmode = 1;  // use RAD mode by default
  dispmode = 0;  // normal display mode by default
  current_calc_prog_config_mode = calc_mode;
  shift = baseLayer;  // base layer
  //precision = 9;
  precision = 6;
  
  mem_recall_mode = false;
  mem_store_mode = false;  
  mem_clear_mode = false;

  set_trigconv(); 

  clear_input();
  clear_stack();
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
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

// function definitions
void enter_number(uint8_t keycode)
{
  if (!input.started)
  {
    if (!input.enter_pressed && !error_flag) 
    {
      stack_push(0); 
      LCDDrawStackAndMem();
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
  LCDDrawInput();
  LCDDrawCalcStatus();
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

void enter_enter(__attribute__ ((unused)) uint8_t keycode)
{
  if (input.started) 
  {
    stack[0] = convert_input();    
    clear_input();
  }
  if (error_flag) return; 
  input.enter_pressed = 1;
  stack_push(stack[0]);
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
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
    case KC_COS: f = cos(trigconv*x); break;
    case KC_TAN: f = tan(trigconv*x); break;
    case KC_ASIN: f = asin(x)/trigconv; break;
    case KC_ACOS: f = acos(x)/trigconv; break;
    case KC_ATAN: f = atan(x)/trigconv; break;
    case KC_SQRT: f = sqrt(x); break;
    case KC_MULTINV: f = 1./x; break;
    case KC_ABS: f = fabs(x); break;
    case KC_SQUARE: f = x*x; break;
    case KC_LN: f = log(x); break;
    case KC_EXP: f = exp(x); break;
    case KC_FACT: f = factorial(x); break;
    case KC_ROUND: f = round(x); break;
    case KC_FRAC: f = x - trunc(x); break;
    case KC_INTEG: f = trunc(x); break;
    /* 
    case OP_LG: f = log10(x); break;
    case OP_POW10: f = pow(10., x); break;    
    */
    default: break;
	}
  lastx = stack[0];  
  stack[0] = f;  
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
} // apply_func_1to1

double factorial(double x)
{
    if (x <= 0.0) return 0;

    double i;
    double result = 1.0;
 
    // loop from 2 to n to get the factorial
    for (i = 2; i <= x; i++) {
        result *= i;
    }
 
    return result;
}

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
    case KC_SUBS: f = y-x; break;
    case KC_MULT: f = y*x; break;
    case KC_DIV: f = y/x; break;    
    case KC_POW: f = pow(y, x); break;
    case KC_ROOT: f = pow(y, 1./x); break;
    default: break;
	}
  lastx = stack[0];  
  stack_drop();
  stack[0] = f;  
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
} // apply_func_2to1

void enter_backspace_clrx(__attribute__ ((unused)) uint8_t keycode) 
{
  if (input.started) 
  {
    if (input.expentry == 0) 
    {
      if (input.mpos > 0) 
      {
        if (input.point>0 && input.point>=input.mpos) 
        {
          input.point = 0;
        } 
        else 
        {
          input.mpos--;
          if (input.mpos == 0) input.sign = 0; 
        }
      } 
      else 
      {
        input.sign = 0;
      }
    } else {
      if (input.exponent[0] == 0 && input.exponent[1] == 0 && input.exponent[2] == 0)
        input.expentry = 0;
      else {
        input.exponent[0] = input.exponent[1];
        input.exponent[1] = input.exponent[2];
        input.exponent[2] = 0;
        if (input.exponent[0] == 0 && input.exponent[1] == 0 && input.exponent[2] == 0)
          input.expsign = 0;
      }
    }
    LCDDrawInput(); 
  } 
  else // input is not started when backspace button is pressed
  {
    error_flag = 0;     
    stack[0] = 0;  // clear x
    LCDDrawStackAndMem(); 
  }
  LCDDrawCalcStatus();
} // enter_backspace()

void enter_decpoint(__attribute__ ((unused)) uint8_t keycode) 
{
  if (!input.started) 
  {    
    if (!input.enter_pressed && !error_flag) 
    { 
      stack_push(0);       
      LCDDrawStackAndMem(); 
    }
    error_flag = 0; 
    input.started = 1;
    input.enter_pressed = 0;    
  }
  if (input.expentry == 0 && input.point == 0) 
  {
    if (input.mpos == 0) input.mantissa[input.mpos++] = 0; 
    input.point = input.mpos; 
  }
  LCDDrawInput(); 
  LCDDrawCalcStatus();
} // enter_decpoint()

void enter_sign(__attribute__ ((unused)) uint8_t keycode) 
{
  if (input.started) {
    if (input.expentry==0) {
    	input.sign = 1-input.sign;
    } else {
      input.expsign = 1-input.expsign; 
    }
    LCDDrawInput(); 
  } else {
    if (error_flag) return; 
    stack[0] = -stack[0];
    LCDDrawStackAndMem(); 
  }
  LCDDrawCalcStatus();
} // enter_sign()

void enter_drop(__attribute__ ((unused)) uint8_t keycode) {
  error_flag = 0; 
  if (input.started) {
    clear_input(); 
  }
  input.enter_pressed = 0;
  stack_drop();
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
}

void enter_swap_xy(__attribute__ ((unused)) uint8_t keycode) {
  if (input.started) {
		stack[0] = convert_input();
	  clear_input();
  }
  if (error_flag) return; 
  input.enter_pressed = 0;
  double tmp = stack[0]; 
  stack[0] = stack[1]; 
  stack[1] = tmp;
  LCDDrawStackAndMem(); 
  LCDDrawCalcStatus();
}

void enter_rotate(uint8_t keycode) {
  if (input.started) {
    stack[0] = convert_input();
	  clear_input();
  }
  if (error_flag) return;
  input.enter_pressed = 0;
  if (keycode == KC_ROLLDN)
  {
    stack_rotate_down();
  } else {
    stack_rotate_up();
  }
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
} // enter_rotate()

void enter_shift_base(__attribute__ ((unused)) uint8_t keycode) {
	shift = baseLayer;
	LCDDrawCalcStatus();
}

void enter_shift_f(__attribute__ ((unused)) uint8_t keycode) {
	shift = fLayer;
	LCDDrawCalcStatus();
}

void enter_shift_g(__attribute__ ((unused)) uint8_t keycode) {
	shift = gLayer;
	LCDDrawCalcStatus();
}

void enter_shift_h(__attribute__ ((unused)) uint8_t keycode) {
	shift = hLayer;
	LCDDrawCalcStatus();
}

void enter_calc_mode(__attribute__ ((unused)) uint8_t keycode)
{
  current_calc_prog_config_mode = calc_mode;
  LCDDrawCalcStatus();
}

void enter_prog_mode(__attribute__ ((unused)) uint8_t keycode)
{
  current_calc_prog_config_mode = prog_mode;
  LCDDrawCalcStatus();
}

void enter_config_mode(__attribute__ ((unused)) uint8_t keycode)
{
  current_calc_prog_config_mode = config_mode;
  // set current_config_screen variable = main_config_screen (enum)
  LCDDrawCalcStatus(); // this should be a call to LCDDrawConfigScreen()
}

void enter_exp(__attribute__ ((unused)) uint8_t keycode) 
{
  if (!input.started) 
  {
    if (!input.enter_pressed && !error_flag) {
      stack_push(0);
      LCDDrawStackAndMem(); 
    }
    error_flag = 0; 
    input.started = 1;
    input.enter_pressed = 0;
  }
  if(input.mpos == 0 || (input.mpos == 1 && input.mantissa[0] == 0)) 
  {
    input.mantissa[0] = 1; 
    input.mpos = 1;
  }
  input.expentry = 1; 
  LCDDrawInput();
  LCDDrawCalcStatus();
} // enter_exp()

void apply_const(uint8_t keycode) {
	if (input.started) {
    stack[0] = convert_input();
		clear_input();
	}
	input.enter_pressed = 0;
	double f = 0.;
	switch(keycode) {
    case KC_PI: f = M_PI; break;
    default: break;
	}

	if (!error_flag) {
	    stack_push(f);
	} else {
	    error_flag = 0;
	    stack[0] = f;
	}
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
} // apply_const()

void enter_lastx(__attribute__ ((unused)) uint8_t keycode) {
  if (input.started) {
    stack[0] = convert_input();
		clear_input();
  }
  input.enter_pressed = 0;
  if (!error_flag) {
    stack_push(lastx);
  } else {
    error_flag = 0; 
    stack[0] = lastx;
  }
  LCDDrawStackAndMem(); 
  LCDDrawCalcStatus();
} // enter_lastx

void enter_clear(__attribute__ ((unused)) uint8_t keycode) {
  error_flag = 0;
  if (input.started) {
    clear_input();
  }
  input.enter_pressed = 0;
  clear_stack();
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
} // enter_clear

// this will be called directly from loop() when calculator is already in mem rcl mode, and a key (variable name to recall) is provided
void apply_memory_rcl(uint8_t r, uint8_t c) {
	if (input.started) {
		stack[0] = convert_input();
    clear_input();
	}
	input.enter_pressed = 0;
	double f = variables[r][c];
	if (!error_flag) {
    stack_push(f);
	} else {
    error_flag = 0;
    stack[0] = f;
	}
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
}

// this will be called directly from loop() when calculator is already in mem sto mode, and a key (variable name to store to) is provided
void apply_memory_sto(uint8_t r, uint8_t c) {
	if (error_flag) return;
	if (input.started) {
		stack[0] = convert_input();
    clear_input();
	}
	input.enter_pressed = 0;
	variables[r][c] = stack[0];
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
}

// this will be called directly from loop() when calculator is already in mem clr mode, and a key (variable name to store to) is provided
void apply_memory_clr(uint8_t r, uint8_t c) {
	if (error_flag) return;
	if (input.started) {
		stack[0] = convert_input();
    clear_input();
	}
	input.enter_pressed = 0;
	variables[r][c] = 0;
  LCDDrawStackAndMem();
  LCDDrawCalcStatus();
}

void toggle_mem_mode(uint8_t keycode)
{
  switch(keycode) 
  {
    case KC_STOREG: mem_store_mode = !mem_store_mode; break;
    case KC_RCLREG: mem_recall_mode = !mem_recall_mode; break;
    case KC_CLRREG: mem_clear_mode = !mem_clear_mode; break;
    default: break;
	}
} // toggle_mem_mode

void set_trig_mode(uint8_t keycode)
{
  switch (keycode)
  {
    case KC_MODERAD: trigmode = 1; break;
    case KC_MODEDEG: trigmode = 0; break;
  }
  set_trigconv();
} // set_trig_mode

void do_nothing(__attribute__ ((unused)) uint8_t keycode)
{}

// print a number to the LCD on given page from a given number_for_lcd struct
void LCDDrawNum(number_for_lcd *nfl, uint8_t page)
{
  uint8_t col = 0;
  
  if (nfl->sign)
  {
    mylcd.LCDChar(MCFMINUS, col, page);
    col += MCFFONTWIDTH + MCFFONTSPACER;
  }
  
  for (uint8_t d = 0; d < nfl->num_digits; d++)
  {
    if (d == nfl->dec_point_pos && nfl->show_dec_point)
    {
      mylcd.LCDDot(col, page);
      col += MCFDECPOINTWIDTH + MCFFONTSPACER;
    }
    
    mylcd.LCDChar(nfl->digits[d], col, page);
    col += MCFFONTWIDTH + MCFFONTSPACER;
    
    int8_t thousandcheck = d + 1 - nfl->dec_point_pos;
    
    if ( (thousandcheck % 3 == 0) && (thousandcheck != 0) )
    {
      col += MCFTHOUSANDSPACER;
    }     
  }
  
  if (nfl->dec_point_pos == nfl->num_digits && nfl->show_dec_point)
  {
    mylcd.LCDDot(col, page);
  }
} // LCDDrawNum

// convert num into an number_for_lcd struct and send it to page by calling the DrawNum() function
// expect LCDNumber() to be called by LCDDrawStack()
void LCDNumber(double num, uint8_t page)
{
  int _exponent; 
  int64_t _mantissa;
  int pointpos=1; 
  if (num != 0) {
    _exponent = (int)floor(log10(fabs(num)));
    _mantissa = (int64_t)round(fabs(num)/pow(10, _exponent-precision+1));
    if (_mantissa > max_mantissa[precision]) {
      _mantissa /= 10;
      _exponent += 1; 
    }
    if (_exponent < precision && _exponent > 0) {
      pointpos = _exponent+1; 
      _exponent = 0; 
    }
    if (_exponent < 0 && _exponent > -4) {
      _mantissa = (int64_t)round(fabs(num)*pow(10, precision-1));
      pointpos = 1; 
      _exponent = 0;
    }
    for (int i=0; i<precision; i++) 
    {
      if (_mantissa % 10 == 0 && pointpos < precision) {
        _mantissa /= 10; 
        pointpos++; 
      } else {
        break;
      }
    } // for
    if (num<0) _mantissa = -_mantissa; 
  } else {
    _mantissa = 0;
    _exponent = 0;
    pointpos = precision;
  }
  
  int64_t mantissa = _mantissa;  // could probably re-merge the 2 by creating a 'isnegative' bool variable
  int32_t exponent = _exponent;  // could probably re-merge the 2, need to clarify int vs int32_t

  if (_mantissa < 0) mantissa = -mantissa;
  number_for_lcd nfl;
  int j;
  for (j=0; j<precision; j++) {
    uint8_t ch = mantissa % 10; 
    mantissa = mantissa/10;
    
    nfl.digits[j] = ch;
    
    if (mantissa == 0 && pointpos + j >= precision) break;
  }
  
  if (_mantissa<0)
  {
    nfl.sign = 1;
  } else 
  {
    nfl.sign = 0;
  }

  nfl.num_digits = j+1;
  nfl.dec_point_pos = nfl.num_digits - precision + pointpos;  
  nfl.show_dec_point = (nfl.dec_point_pos < nfl.num_digits);
  
  //mylcd.LCDChar(pointpos, 14*10, page);  // debugging print-out
  //mylcd.LCDChar(nfl.num_digits, 14*12, page);  // debugging print-out

  /** need to implement this for scientific notation presentation of numbers
  if (exponent != 0) {
    draw_char(0, pos, 0, 1);  // convert this to a write to the number_for_lcd variable
    if (exponent<0) draw_char(1, pos, '-', 1);  // convert this to a write to the number_for_lcd variable
    if (exponent<0) exponent = -exponent;
    for (int i=0; i<3; i++) {
      uint8_t ch = exponent % 10; 
      exponent = exponent/10;
      draw_char((4-i), pos, ch+'0', 1);  // convert this to a write to the number_for_lcd variable
    }
  }
  **/

  // reverse the 'digits' array so as to be ready for LCDDrawNum function
  int l=0;
  int r = j;
  while (l < r)
  {
    uint8_t temp;
    temp = nfl.digits[l];
    nfl.digits[l] = nfl.digits[r];
    nfl.digits[r] = temp;
    l++;
    r--;
  }
  
  //mylcd.LCDChar(nfl.sign, 14*0, 0 * 3);
  //mylcd.LCDChar(nfl.num_digits, 14*2, 0 * 3);
  
  LCDDrawNum(&nfl, page);
}  // LCDNumber()

// call LCDNumber for all of the stack and mem registers that we want to show on screen
// expect this function to be called from apply_func(), etc.
void LCDDrawStackAndMem()
{
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  
  LCDNumber(stack[0], XLCDPAGE);
  LCDNumber(stack[1], YLCDPAGE);
  LCDNumber(stack[2], ZLCDPAGE);
}

// convert 'input' structure into a number_for_lcd struct and send it to page X by calling the DrawNum() function
// expect LCDDrawInput to be called by enter_number() function (for example)
void LCDDrawInput()
{
  // clear X pages before re-drawing
  mylcd.LCDGotoXY(0, XLCDPAGE);
  mylcd.LCDFillPage(0x00); // Clear page
  mylcd.LCDFillPage(0x00); // Clear page
  
  // create a number_for_lcd (local) variable, to be build from the input (global) variable
  number_for_lcd nfl;
  nfl.num_digits = input.mpos;
  for (int d = 0; d < nfl.num_digits; d++)
  {
    nfl.digits[d] = input.mantissa[d];
  }
  
  nfl.sign = input.sign;  
  nfl.dec_point_pos = (input.point == 0) ? input.mpos : input.point;
  nfl.show_dec_point = (input.point > 0);
  
  // draw the input in the X page
  LCDDrawNum(&nfl, XLCDPAGE);
} // LCDDrawInput()


void testStillNumbers2()
{
  stack[0] = 2.25;
  stack[1] = 1024;
  stack[2] = 56000000;
  
  LCDDrawStackAndMem();
  
  delay(10000);
}

void testActionsWithoutKeyboard()
{
 
  // print keypos here on 'Z' page
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  clear_input();
  clear_stack();
  
  //mylcd.LCDChar('R' - MCFLETOFFSET, 14*0, 0*3);  // R
  //mylcd.LCDChar(keypos_r, 14*1, 0 * 3);
  //mylcd.LCDChar('C' - MCFLETOFFSET, 14*3, 0*3); // C 
  //mylcd.LCDChar(keypos_c, 14*4, 0 * 3);  
  //delay(5000);
  
  // hard code key press
  keypos_r = 0x00; keypos_c = 0x07;
  
  // convert key press into an action struct
  action current_action;
  current_action = keytoaction();
  // print keycode here on 'Y' page
  //mylcd.LCDString("KC", 14*0, 0*3);
  //mylcd.LCDCharSeq(current_action.mnemonic, 1, 14*3, 1 * 3);
  mylcd.LCDChar((current_action.keycode % 1000) / 100, 14 * 9, 0*3);
  mylcd.LCDChar((current_action.keycode % 100) / 10, 14 * 10, 0*3);
  mylcd.LCDChar(current_action.keycode % 10, 14 * 11, 0*3);
  delay(2000);
  
  // execute the action
  current_action.fct(current_action.keycode);  

  // more actions
  //ACT_ENTER.fct(ACT_ENTER.keycode);
  keypos_r = 0x00; keypos_c = 0x01;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  
  keypos_r = 0x02; keypos_c = 0x07;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  
  //ACT_8.fct(ACT_8.keycode); 
  keypos_r = 0x00; keypos_c = 0x08;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  
  //ACT_ADD.fct(ACT_ADD.keycode);
  keypos_r = 0x02; keypos_c = 0x01;
  current_action = keytoaction();
  current_action.fct(current_action.keycode);
  
  //mylcd.LCDString("DONE", 0, YLCDPAGE);
  delay(2000);
  
  // draw results on LCD
  LCDDrawStackAndMem();
  delay(2000);
  
  // enter
  keypos_r = 0x00; keypos_c = 0x01;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  
  LCDDrawStackAndMem();
  delay(2000);
  
  // sqrt 
  keypos_r = 0x03; keypos_c = 0x02;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  
  LCDDrawStackAndMem();
  delay(2000);
} // testActionsWithoutKeyboard()

void simulateInput()
{
  mylcd.LCDFillScreen(0x00, 0); // clear screen
  clear_input();
  clear_stack();
  
  action current_action;
  
  LCDDrawStackAndMem();
  delay(2000);
  
  // 8
  keypos_r = 0x00; keypos_c = 0x08;
  current_action = keytoaction();
  current_action.fct(current_action.keycode); 
  LCDDrawInput();
  delay(2000);
  
  // 7
  keypos_r = 0x00; keypos_c = 0x07;
  current_action = keytoaction();
  current_action.fct(current_action.keycode);
  LCDDrawInput();
  delay(2000);
  
  // 6
  keypos_r = 0x01; keypos_c = 0x09;
  current_action = keytoaction();
  current_action.fct(current_action.keycode);
  LCDDrawInput();
  delay(2000);
  
  // enter
  keypos_r = 0x00; keypos_c = 0x01;
  current_action = keytoaction();
  current_action.fct(current_action.keycode);
  LCDDrawStackAndMem();
  delay(2000);  
  
} // simulateInput()

//void LCDDrawCalcStatus() {}

// TO DO: increase the size of the symbols from 6x6 pixels to 12x12 pixels instead, with a max of 5 symbols
void LCDDrawCalcStatus()
{
  // indicator area 1, degree mode
  switch (trigmode)
  {
    case 0:
      mylcd.LCDBitmap(180, 13*0, 6, 8, MiyoCalcFont_Degree);  // degrees
      break;
    case 1:
    default:
      mylcd.LCDBitmap(180, 13*0, 6, 8, MiyoCalcFont_Blank);  // radian
  } 
  
  // indicator area 2, shift layer
  switch (shift) 
  {
    case fLayer:
      mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_ShiftF); // f layer
      break;
    case gLayer:
      mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_ShiftG); // g layer
      break;
    case hLayer:
      mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_ShiftH); // h layer
      break;
  }
  
  // indicator 2, register store, recall, clear mode
  if (mem_store_mode)
  {
    mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_RegSto); 
  } 
  else if (mem_recall_mode)
  {
    mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_RegRcl); 
  } 
  else if (mem_clear_mode)
  {
    mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_RegClr); 
  }  
  
  if (shift == baseLayer && !mem_store_mode && !mem_recall_mode && !mem_clear_mode)
  {
     mylcd.LCDBitmap(180, 13*1, 6, 8, MiyoCalcFont_Blank); // blank
  }
  
} // LCDDrawCalcStatus()

