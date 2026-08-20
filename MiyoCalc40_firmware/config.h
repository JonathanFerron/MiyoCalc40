/* 'config mode' stuff

*/
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// screens reachable within 'config mode'. main_cfg_screen, battvolt_cfg_screen, and
// backlight_cfg_screen are implemented so far -- contrast_cfg_screen etc. are the next ones to
// land, per the config layer layout (see the comment above config_card in cards.cpp).
enum config_screens
{ main_cfg_screen = 0,
  battvolt_cfg_screen,
  backlight_cfg_screen
};

extern uint8_t current_config_screen;

// function prototypes
void enter_config_mode(uint8_t keycode);
void config_show_battvolt(uint8_t keycode);
void config_show_backlight(uint8_t keycode);
void config_adjust(uint8_t keycode);
void config_cancel_exit(uint8_t keycode);
void LCDDrawConfigScreen();

#endif
