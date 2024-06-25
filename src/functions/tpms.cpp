#include "tpms.h"
#include "ui/ui.h"

#define FRONT_PRESSURE_LOW 34
#define FRONT_PRESSURE_HIGH 40
#define FRONT_TEMPERATURE_LOW 40
#define FRONT_TEMPERATURE_HIGH 50

#define BACK_PRESSURE_LOW 40
#define BACK_PRESSURE_HIGH 46
#define BACK_TEMPERATURE_LOW 40
#define BACK_TEMPERATURE_HIGH 50

void setColor(String color, String position)
{
  lv_obj_t *position_element;
  
  if (position == "Front_Pressure")
  {
    position_element = ui_Front_Pressure;
  }
  else if (position == "Front_Temperature")
  {
    position_element = ui_Front_Temperature;
  }
  else if (position == "Back_Pressure")
  {
    position_element = ui_Back_Pressure;
  }
  else
  {
    position_element = ui_Back_Temperature;
  }
  
  if (color == "red")
  {
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_Red);
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_Red);
  }
  else if (color == "blue")
  {
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_Blue);
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_Blue);
  }
  else
  {
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
                                           _ui_theme_color_Green);
    ui_object_set_themeable_style_property(position_element, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA,
                                           _ui_theme_alpha_Green);
  }
}

void updateFrontTyreValues(int battery, float pressure, float temperature)
{
  // battery value needs to have the % sign
  lv_label_set_text(ui_Front_Battery_Value, String(String(battery) + "%").c_str());
  lv_label_set_text(ui_Front_Pressure_Value, String(pressure).substring(0,4).c_str());
  lv_label_set_text(ui_Front_Temperature_Value, String(temperature).substring(0,4).c_str());
  // set color of the pressure value
  // if pressure is above 40 psi, set the color to red
  if (pressure > FRONT_PRESSURE_HIGH)
  {
    setColor("red", "Front_Pressure");
  }
  // if pressure is below 34 psi, set the color to blue
  else if (pressure < FRONT_PRESSURE_LOW)
  {
    setColor("blue", "Front_Pressure");
  }
  // if pressure is between 34 and 40 psi, set the color to green
  else
  {
    setColor("green", "Front_Pressure");
  }

  // set color of the temperature value
  // if temperature is above 40 degrees, set the color to red
  if (temperature > FRONT_TEMPERATURE_HIGH)
  {
    setColor("red", "Front_Temperature");
  }
  // if temperature is below 34 degrees, set the color to blue
  else if (temperature < FRONT_TEMPERATURE_LOW)
  {
    setColor("blue", "Front_Temperature");
  }
  // if temperature is between 34 and 40 degrees, set the color to green
  else
  {
    setColor("green", "Front_Temperature");
  }
}

void updateBackTyreValues(int battery, float pressure, float temperature)
{
  lv_label_set_text(ui_Back_Battery_Value, String(String(battery) + "%").c_str());
  lv_label_set_text(ui_Back_Pressure_Value, String(pressure).substring(0,4).c_str());
  lv_label_set_text(ui_Back_Temperature_Value, String(temperature).substring(0,4).c_str());

  // set color of the pressure value
  // if pressure is above 46 psi, set the color to red
  if (pressure > BACK_PRESSURE_HIGH)
  {
    setColor("red", "Back_Pressure");
  }
  // if pressure is below 40 psi, set the color to blue
  else if (pressure < BACK_PRESSURE_LOW)
  {
    setColor("blue", "Back_Pressure");
  }
  // if pressure is between 34 and 40 psi, set the color to green
  else
  {
    setColor("green", "Back_Pressure");
  }

  // set color of the temperature value
  // if temperature is above 50 degrees, set the color to red
  if (temperature > BACK_TEMPERATURE_HIGH)
  {
    setColor("red", "Back_Temperature");
  }
  // if temperature is below 40 degrees, set the color to blue
  else if (temperature < BACK_TEMPERATURE_LOW)
  {
    setColor("blue", "Back_Temperature");
  }
  // if temperature is between 34 and 40 degrees, set the color to green
  else
  {
    setColor("green", "Back_Temperature");
  }
}
