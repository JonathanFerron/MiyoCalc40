/*
  config mode code for:
  - battery voltage display
  - screen contrast (delegate most of this to the lcd.c file)
  - backlight front end (delegate most to backlight.c, use left & right for off and on)
  - max stack size (4 to 8) and mode: fixed depth, aka traditional hp rpn, or 'normal' stack that grows and shrinks from a minimum size of 1
  - 1 or 2 column
  - import & export programs from/to progmem (then user key any of 36 program key, when keys for unique program are pressed)
  - key debouncing delays / parameters
  - soft reset

  Config could be stored in eeprom.

  when 'backlight' button is keyed from main cfg screen:
  toggle 'current cfg screen' to 'backlight'
  draw config screen

  when up, down, left and right arrow (special KCC_ non-programmable key codes, use separate defines, e.g. KCC_UP) are pressed in config mode,
  call "config_adjust(keycode)" function

  config_adjust:
  switch (current cfg screen)
    case backlight
      call adj_bl(keycode)
      draw cfg screen

*/

#include "config.h"
#include "cards.h"
#include "calc.h"
#include "main.h"
#include "fonts.h"
#include "util.h"
#include "backlight.h"

uint8_t current_config_screen;

static void LCDDrawConfigMain()
{ mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDString("CONFIG", 0, ZLCDPAGE);
}

static void LCDDrawConfigBattVolt()
{ mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDString("BATT", 0, ZLCDPAGE);

  uint16_t mv = read_batt_voltage_mv();

  number_for_lcd nfl;
  nfl.digits[0] = mv / 1000;
  nfl.digits[1] = (mv % 1000) / 100;
  nfl.digits[2] = (mv % 100) / 10;
  nfl.digits[3] = mv % 10;
  nfl.sign = 0;
  nfl.num_digits = 4;
  nfl.dec_point_pos = 1;
  nfl.show_dec_point = true;
  nfl.show_exponent = false;
  nfl.exponent = 0;
  LCDDrawNum(&nfl, YLCDPAGE);

  mylcd.LCDChar('V' - MCFLETOFFSET, 65, YLCDPAGE);
} // LCDDrawConfigBattVolt()

static void LCDDrawConfigBacklight()
{ mylcd.LCDFillScreen(0x00, 0); // clear screen
  mylcd.LCDString("BKLT", 0, ZLCDPAGE);

  uint8_t level = backlight_get(); // notch index, 0..BACKLIGHT_TABLE_SIZE-1

  number_for_lcd nfl;
  nfl.digits[0] = level / 10;
  nfl.digits[1] = level % 10;
  nfl.sign = 0;
  nfl.num_digits = 2;
  nfl.dec_point_pos = 2;
  nfl.show_dec_point = false;
  nfl.show_exponent = false;
  nfl.exponent = 0;
  LCDDrawNum(&nfl, YLCDPAGE);
} // LCDDrawConfigBacklight()

void LCDDrawConfigScreen()
{ switch(current_config_screen)
  { case battvolt_cfg_screen:
      LCDDrawConfigBattVolt();
      break;
    case backlight_cfg_screen:
      LCDDrawConfigBacklight();
      break;
    case main_cfg_screen:
    default:
      LCDDrawConfigMain();
      break;
  }
  // every branch above starts with a full-screen LCDFillScreen(), which wipes the area-3 mode
  // indicator along with everything else -- redraw it here so config mode still shows its glyph.
  LCDDrawCalcStatus();
} // LCDDrawConfigScreen()

// entry point into 'config mode', reached from the calc-mode h layer (see ACT_CFG_MOD in cards.cpp)
void enter_config_mode(__attribute__((unused)) uint8_t keycode)
{ current_calc_prog_config_mode = config_mode;
  current_config_screen = main_cfg_screen;
  LCDDrawConfigScreen();
} // enter_config_mode()

// 'batt volt' key on the config layer (r3,c4): switch to the battery-voltage screen and take a
// reading. Pressing it again while already on that screen just re-reads (manual refresh).
void config_show_battvolt(__attribute__((unused)) uint8_t keycode)
{ current_config_screen = battvolt_cfg_screen;
  LCDDrawConfigScreen();
} // config_show_battvolt()

// 'back light' key on the config layer (r3,c2): switch to the backlight screen.
void config_show_backlight(__attribute__((unused)) uint8_t keycode)
{ current_config_screen = backlight_cfg_screen;
  LCDDrawConfigScreen();
} // config_show_backlight()

// up/down jump straight to full on/off; left/right step one notch at a time through
// backlight.cpp's perceptual brightness table, matching the "up and down keys to turn on/off,
// right/left keys to increase/decrease brightness" note above config_card in cards.cpp
static void adj_backlight(uint8_t keycode)
{ switch(keycode)
  { case KCC_UP:
      backlight_set(BACKLIGHT_TABLE_SIZE - 1);
      break;
    case KCC_DOWN:
      backlight_set(0);
      break;
    case KCC_LEFT:
      backlight_step(false);
      break;
    case KCC_RIGHT:
      backlight_step(true);
      break;
    default:
      break; // not a direction key
  }
} // adj_backlight()

// dispatch for the up/down/left/right keys (KCC_UP/DOWN/LEFT/RIGHT in cards.h), shared across
// every config screen that has adjustable settings -- only backlight_cfg_screen uses them so far.
void config_adjust(uint8_t keycode)
{ switch(current_config_screen)
  { case backlight_cfg_screen:
      adj_backlight(keycode);
      break;
    default:
      break;
  }
  LCDDrawConfigScreen();
} // config_adjust()

// 'cancel/exit' key on the config layer (r3,c8): back out one level.
void config_cancel_exit(uint8_t keycode)
{ switch(current_config_screen)
  { case main_cfg_screen:
      enter_calc_mode(keycode);
      break;
    default:
      current_config_screen = main_cfg_screen;
      LCDDrawConfigScreen();
      break;
  }
} // config_cancel_exit()
