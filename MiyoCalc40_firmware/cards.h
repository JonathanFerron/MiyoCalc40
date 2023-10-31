// function prototypes for cards.c
/*
#ifdef __cplusplus
  extern "C" {
#endif
*/
#ifndef CARDS_H
#define CARDS_H

// define all keycodes (maximum of 256 = 0xFF)

// currently only using 95 out of the maximum of 256 keycodes

// first group is for entering constants in the X variable (bottom of the stack)
#define KC_0           0x00  // it's important to leave the keycodes for the numbers unchanged, or else the 'enter_number()' function needs to be made more robust
#define KC_1           0x01
#define KC_2           0x02
#define KC_3           0x03
#define KC_4           0x04
#define KC_5           0x05
#define KC_6           0x06
#define KC_7           0x07
#define KC_8           0x08
#define KC_9           0x09
#define KC_A           0x0A  // this is when in HEX mode
#define KC_B           0x0B
#define KC_C           0x0C
#define KC_D           0x0D
#define KC_E           0x0E
#define KC_F           0x0F

#define KC_POINT       0x10  // .
#define KC_CHGSGN      0x11  // + and -
#define KC_EEX         0x12  // scientific notation entry
#define KC_PI          0x13  // pi constant (3.14)

// group for fundamental operations
#define KC_ADD	       0x30  // +
#define KC_SUBS	       0x31  // -
#define KC_MULT	       0x32  // *
#define KC_DIV	       0x33	 // /
#define KC_POW		     0x34  // y^x
#define KC_ROOT		     0x35  // y^(1/x)
#define KC_MULTINV	   0x36	 // 1/x
#define KC_ABS	       0x37	
#define KC_SQRT		     0x38  // square root
#define KC_SQUARE		   0x39  // x * x
#define KC_LN		       0x3A  // natural log
#define KC_EXP		     0x3B  // e^x
#define KC_FACT		     0x3C  // factorial
#define KC_ROUND	     0x3D	
#define KC_FRAC		     0x3E  // fractional part
#define KC_INTEG	     0x3F  // integer part

// group for stack manipulation
#define KC_ENTER	     0x50
#define KC_DROPX	     0x51	 // drop
#define KC_SWAP	       0x52	 // swap x and y
#define KC_ROLLDN	     0x53	 // rolldown
#define KC_ROLLUP	     0x54	 // roolup
#define KC_BKSCLRX     0x55  // backspace and clear x
#define KC_LASTX	     0x56	
#define KC_CLRSTCK	   0x57  // clear stack

// group for memory registers manipulation
#define KC_STOREG	     0x58
#define KC_RCLREG	     0x59
#define KC_CLRREG	     0x5A

// group for trig functions
#define KC_SIN	       0x70
#define KC_COS	       0x71
#define KC_TAN	       0x72	
#define KC_ASIN	       0x73	
#define KC_ACOS	       0x74	
#define KC_ATAN	       0x75

// group for bitwise operations
#define KC_BWAND	     0x90
#define KC_BWOR		     0x91
#define KC_BWXOR	     0x92	
#define KC_BWNOT	     0x93

// group for number representation mode
#define KC_MODEBIN	   0x94
#define KC_MODEOCT	   0x95	
#define KC_MODEDEC	   0x96	
#define KC_MODEHEX	   0x97

// group for angle representation mode
#define KC_MODERAD	   0x98  // radian
#define KC_MODEDEG	   0x99  // degrees

// other useful built-in functions: 0xB0 to 0xCF
#define KC_DATE	       0xB0  // given date + cst (days) = what date 
#define KC_DATEDIFF	   0xB1  // diff between two dates in days

// programming specific keycodes: 0xD0 to 0xEF
#define KCP_IF	       0xD0
#define KCP_ELSE	     0xD1	
#define KCP_ENDIF	     0xD2		
#define KCP_CASE	     0xD3		
#define KCP_WHEN	     0xD4		
#define KCP_OTHER		   0xD5	
#define KCP_ENDCAS	   0xD6	 // end case
#define KCP_SETFLAG	   0xD7		
#define KCP_CLRFLAG	   0xD8		
#define KCP_FOR		     0xD9	
#define KCP_NEXT	     0xDA		
#define KCP_WHILE		   0xDB	
#define KCP_WEND	     0xDC		
#define KCP_DO	       0xDD		
#define KCP_LOOPWHEN	 0xDE	

#define KCP_XEQU0	     0xE0  // x=0
#define KCP_XNOT0		   0xE1  // x not equal to 0
#define KCP_XLESS0	   0xE2	 // x < 0
#define KCP_XLEEQ0	   0xE3	 // x <= 0
#define KCP_XGREA0	   0xE4	 // x > 0
#define KCP_GREQ0	     0xE5	 // x >= 0

#define KCP_XEQUY      0xE6  // x = y
#define KCP_CEQUN	     0xE6  // cnt = n

#define KCP_XNOTY	     0xE7  // x <> y
#define KCP_CNOTN	     0xE7	

#define KCP_XLESSY	   0xE8	
#define KCP_CLESSN	   0xE8

#define KCP_XLEEQY	   0xE9		
#define KCP_CLEEQN	   0xE9		

#define KCP_XGREAY	   0xEA	
#define KCP_CGREAN	   0xEA	

#define KCP_XGREQY	   0xEB		
#define KCP_CGREQN	   0xEB	

#define KCP_CYCLE	     0xEC	 
#define KCP_EXIT 	     0xED		

// special keycodes: 0xF0 to 0xFF
#define KC_NOP         0xFF  // no operation

  // end of keycode definitions
  
#define ONOFFKEYPOS_R 0x03
#define ONOFFKEYPOS_C 0x04

// structures
typedef struct _action
{
  uint8_t keycode;  // e.g. KC_ADD, to be used in programming mode as the keycode to record: if it's an action that's not programmable, use KC_NOP
  void (*fct)(uint8_t keycode_arg);  // function pointer to the actual function to execute (if any), e.g. apply_func_1to1()
  uint8_t mnemonic[5]; // 1 to 5 indexes into the font table (0 to 47). Use MCFNULCHAR for empty characters. This is to display in 'programming' mode.
  // may want to add an 'opmode' variable here (define possible values in an enum) to indicate the type of action (actions that modify x only, actions that do something with x and y and drop one element from stack, etc.)
} action;

enum layers
{
  baseLayer = 0,
  fLayer,
  gLayer,
  hLayer
};

enum calc_prog_config_modes
{
  calc_mode = 0,
  prog_mode,
  config_mode
};

// function prototypes
action keytoaction();

extern const action ACT_SHFT_F;
extern const action ACT_SHFT_G;
extern const action ACT_SHFT_H;

#endif

/*
#ifdef __cplusplus
  }
#endif
*/
