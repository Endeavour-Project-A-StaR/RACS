#pragma once

#include "types.h"

const float SERVO_CENTER = 90.0f;
const float SERVO_LIMIT_MAX = 30.0f;

void nav_rst_integral();
void nav_update_pid(FltData_t *fltdata, float dt);
