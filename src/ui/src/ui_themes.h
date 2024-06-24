// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.1
// LVGL version: 8.3.6
// Project name: ESP32TPMS

#ifndef _UI_THEMES_H
#define _UI_THEMES_H

#ifdef __cplusplus
extern "C" {
#endif

#define UI_THEME_COLOR_RED 0
#define UI_THEME_COLOR_BLUE 1
#define UI_THEME_COLOR_GREEN 2

#define UI_THEME_DEFAULT 0

extern const ui_theme_variable_t _ui_theme_color_Red[1];
extern const ui_theme_variable_t _ui_theme_alpha_Red[1];

extern const ui_theme_variable_t _ui_theme_color_Blue[1];
extern const ui_theme_variable_t _ui_theme_alpha_Blue[1];

extern const ui_theme_variable_t _ui_theme_color_Green[1];
extern const ui_theme_variable_t _ui_theme_alpha_Green[1];

extern const uint32_t * ui_theme_colors[1];
extern const uint8_t * ui_theme_alphas[1];
extern uint8_t ui_theme_idx;

void ui_theme_set(uint8_t theme_idx);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif