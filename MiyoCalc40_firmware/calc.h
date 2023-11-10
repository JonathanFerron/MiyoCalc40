/*
 * Regular (non-programming) calculator functionality (i.e. 'calc mode' stuff)
 */

#include <stdint.h>

/*#ifdef __cplusplus
  extern "C" {
#endif
*/
#ifndef CALC_H
  #define CALC_H
  
  #define ZLCDPAGE 0
  #define YLCDPAGE 3
  #define XLCDPAGE 6
  
  // number_for_lcd struct typedef
  // to do: add the 'exponent' component
  typedef struct {
    uint8_t digits[9];      // Mantissa digits array
    uint8_t sign;           // Sign: 0 for "+", 1 for "-"
    uint8_t dec_point_pos;  // Decimal point position, min 0, max 9 : number of digits to the left of the dec point
    uint8_t num_digits;     // Number of mantissa digits entered, max 9
    bool show_dec_point;
  } number_for_lcd;
  
  extern bool mem_recall_mode;
  extern bool mem_store_mode;
  extern bool mem_clear_mode;
  
  extern int shift;
  
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

  
  extern t_input input;

  // function prototypes
  void calc_init();
  void do_nothing(uint8_t keycode);
  void enter_number(uint8_t keycode);
  void enter_enter(uint8_t keycode);
  void enter_decpoint(uint8_t keycode);
  void enter_backspace_clrx(uint8_t keycode);
  void enter_sign(uint8_t keycode);
  void enter_drop(uint8_t keycode);
  void enter_swap_xy(uint8_t keycode);
  void enter_rotate(uint8_t keycode);
  void apply_func_1to1(uint8_t keycode);
  void apply_func_2to1(uint8_t keycode);
  void enter_shift_base(uint8_t keycode);
  void enter_shift_f(uint8_t keycode);
  void enter_shift_g(uint8_t keycode);
  void enter_shift_h(uint8_t keycode);
  void power_down(uint8_t keycode);
  void enter_exp(uint8_t keycode);
  void apply_const(uint8_t keycode);
  void enter_lastx(uint8_t keycode);
  void enter_clear(uint8_t keycode);
  void enter_calc_mode(uint8_t keycode);
  void enter_config_mode(uint8_t keycode);
  void enter_prog_mode(uint8_t keycode);
  
  void apply_memory_rcl(uint8_t r, uint8_t c);
  void apply_memory_sto(uint8_t r, uint8_t c);
  void apply_memory_clr(uint8_t r, uint8_t c);
  void toggle_mem_mode(uint8_t keycode);
  void set_trig_mode(uint8_t keycode);
  
  void testStillNumbers2();
  void testActionsWithoutKeyboard();
  void simulateInput();
  double factorial(double x);
  
  void LCDDrawStackAndMem();
  void LCDDrawInput();
  void LCDDrawCalcStatus();

#endif

/*
#ifdef __cplusplus
  }
#endif
*/
