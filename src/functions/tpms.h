#ifndef TPMS_H
#define TPMS_H

#include<Arduino.h>
#include "ui/ui.h"

void setColor(String color, String position);
void updateFrontTyreValues(int battery, float pressure, float temperature);
void updateBackTyreValues(int battery, float pressure, float temperature);

#endif