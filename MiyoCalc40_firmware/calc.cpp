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

// Exact powers of ten (all exactly representable in a 64-bit double up to 1e22;
// 1e18 is more than enough headroom for scale10()'s chaining below).
static const double p10[19] = {
		1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,
		1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18
};

// a * 10^k using only exact multiply/divide by table entries, avoiding pow()'s
// exp(y*log(x)) approximation (which is what previously introduced display noise)
static double scale10(double a, int k)
{
	while (k > 18)  { a *= p10[18]; k -= 18; }
	while (k < -18) { a /= p10[18]; k += 18; }
	return (k >= 0) ? a * p10[k] : a / p10[-k];
} // scale10()


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
  // double is 32-bit (float, ~7.2 honest decimal digits) on this toolchain unless
  // -mdouble=64 is forced via platform.local.txt -- do NOT re-enable that flag without
  // re-verifying log10()/floor()/round() on real hardware first: avr-gcc 14.3.0 /
  // avr-libc 2.2.1's libf7-based 64-bit math produced garbage exponents at runtime
  // (e.g. LCDNumber(3.0) computed _exponent ~ -22003) despite linking without errors.
  // See CLAUDE.md.
  precision = 7;
  
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

  number = scale10(number, exponent);
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
  LCDDrawStackAndMem();      // also clears the screen of whatever mode we're coming from
  if (input.started)
  {
    LCDDrawInput();
  }
  LCDDrawCalcStatus();
}

void enter_prog_mode(__attribute__ ((unused)) uint8_t keycode)
{
  current_calc_prog_config_mode = prog_mode;
  LCDDrawCalcStatus();
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

    if (!nfl->show_exponent)  // thousands grouping only makes sense in fixed-point notation
    {
      int8_t thousandcheck = d + 1 - nfl->dec_point_pos;

      if ( (thousandcheck % 3 == 0) && (thousandcheck != 0) )
      {
        col += MCFTHOUSANDSPACER;
      }
    }
  }

  if (nfl->dec_point_pos == nfl->num_digits && nfl->show_dec_point)
  {
    mylcd.LCDDot(col, page);
  }

  if (nfl->show_exponent)
  {
    mylcd.LCDChar(MCFLETTER_E, col, page);
    col += MCFFONTWIDTH + MCFFONTSPACER;

    int16_t exp = nfl->exponent;
    if (exp < 0)
    {
      mylcd.LCDChar(MCFMINUS, col, page);
      col += MCFFONTWIDTH + MCFFONTSPACER;
      exp = -exp;
    }

    // sized to int16_t's full digit range (up to 5 digits) rather than the ~3 digits
    // a legitimate double exponent needs, so a miscomputed exponent can never overflow
    // this buffer and corrupt the stack
    uint8_t expdigits[5];
    uint8_t nexp = 0;
    do {
      expdigits[nexp++] = exp % 10;
      exp /= 10;
    } while (exp != 0 && nexp < sizeof(expdigits));

    while (nexp > 0)
    {
      mylcd.LCDChar(expdigits[--nexp], col, page);
      col += MCFFONTWIDTH + MCFFONTSPACER;
    }
  }
} // LCDDrawNum

// convert num into an number_for_lcd struct and send it to page by calling the DrawNum() function
// expect LCDNumber() to be called by LCDDrawStack()
void LCDNumber(double num, uint8_t page)
{
  // Scientific-notation mantissas use fewer significant digits than fixed-point
  // ('precision', normally 7) to leave screen width for the "E<exponent>" suffix.
  const int SCI_DIGITS = 5;

  int _exponent;
  int32_t _mantissa;
  int pointpos=1;
  int ndigits = precision;
  bool show_exponent = false;

  if (num != 0) {
    double a = fabs(num);
    // log10() is only an estimate of the exponent: it can be off by one right at
    // exact powers of ten, so both directions are corrected below by comparing
    // the scaled mantissa against the [10^(precision-1), 10^precision) window.
    _exponent = (int)floor(log10(a));
    double scaled = scale10(a, precision-1-_exponent);
    if (scaled >= p10[precision]) {
      _exponent += 1;
      scaled = scale10(a, precision-1-_exponent);
    } else if (scaled < p10[precision-1]) {
      _exponent -= 1;
      scaled = scale10(a, precision-1-_exponent);
    }
    _mantissa = (int32_t)round(scaled);
    if (_mantissa >= (int32_t)p10[precision]) {  // rounding pushed it to the next power of ten
      _mantissa /= 10;
      _exponent += 1;
    }

    if (_exponent > -4 && _exponent < precision) {
      // fixed point: window covers 0.0001 <= |num| < 10^precision
      if (_exponent > 0) {
        // integer part spanning multiple digits
        pointpos = _exponent+1;
        _exponent = 0;
      } else if (_exponent < 0) {
        // small decimal (0.0001 <= |num| < 1)
        _mantissa = (int32_t)round(scale10(a, precision-1));
        pointpos = 1;
        _exponent = 0;
      }
      // else _exponent == 0 (1 <= |num| < 10): pointpos already 1, _exponent already 0
    } else {
      // outside the fixed-point window: scientific notation, recomputed at SCI_DIGITS
      // significant digits (same over/underflow correction as the block above)
      double sci_scaled = scale10(a, SCI_DIGITS-1-_exponent);
      if (sci_scaled >= p10[SCI_DIGITS]) {
        _exponent += 1;
        sci_scaled = scale10(a, SCI_DIGITS-1-_exponent);
      } else if (sci_scaled < p10[SCI_DIGITS-1]) {
        _exponent -= 1;
        sci_scaled = scale10(a, SCI_DIGITS-1-_exponent);
      }
      _mantissa = (int32_t)round(sci_scaled);
      if (_mantissa >= (int32_t)p10[SCI_DIGITS]) {
        _mantissa /= 10;
        _exponent += 1;
      }
      pointpos = 1;
      ndigits = SCI_DIGITS;
      show_exponent = true;
    }

    for (int i=0; i<ndigits; i++)
    {
      if (_mantissa % 10 == 0 && pointpos < ndigits) {
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
    pointpos = ndigits;
  }

  int32_t mantissa = _mantissa;  // could probably re-merge the 2 by creating a 'isnegative' bool variable

  if (_mantissa < 0) mantissa = -mantissa;
  number_for_lcd nfl;
  int j;
  for (j=0; j<ndigits; j++) {
    uint8_t ch = mantissa % 10;
    mantissa = mantissa/10;

    nfl.digits[j] = ch;

    if (mantissa == 0 && pointpos + j >= ndigits) break;
  }

  if (_mantissa<0)
  {
    nfl.sign = 1;
  } else
  {
    nfl.sign = 0;
  }

  nfl.num_digits = j+1;
  nfl.dec_point_pos = nfl.num_digits - ndigits + pointpos;
  nfl.show_dec_point = (nfl.dec_point_pos < nfl.num_digits);
  nfl.show_exponent = show_exponent;
  nfl.exponent = (int16_t)_exponent;

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
  nfl.show_exponent = false;
  nfl.exponent = 0;

  // draw the input in the X page
  LCDDrawNum(&nfl, XLCDPAGE);
} // LCDDrawInput()


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

