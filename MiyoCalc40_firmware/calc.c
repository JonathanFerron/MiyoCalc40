/*
start by copying a lot of code from OpenRPNCalc, go through everything, simplify
and modify as necessary in the context of the 'cards', actions', 'keycodes',
and variable size (max 6) stack that grows and shrinks.

openrpncalc pseudocode and structure (from calc.c):
  setup_calc():
    trigmode = false
    dispmode = false
    shift = false
    precision = 10
    call set_trigconv
    call clear_input
    call clear_stack
    call draw_status
    call draw stack

  apply_op(): is called to trigger the execution of the actions
    this calls the apply_enter, apply_stack, apply_const, apply_memory_rcl,
    apply_memory_sto, convert_input, clear_input, apply_func_1to1,
    apply_func_2to1, applyfunc2to2, draw_status and
    draw_stack functions (anything that starts with 'draw' should probably go
    in our lcd.c). code depends on the use of the 'op mode' portion of the
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
  variables: array of double (use single to start with), storage space for variables


dcalc pseudocode and structure:


repocalc pseudocode and structure: 



*/