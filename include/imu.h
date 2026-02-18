#pragma once

#include "types.h"

bool imu_init();
bool imu_read(FltData_t* fltdata);
void imu_gyro_cal(FltData_t* fltdata);
void imu_run_fusion(FltData_t* fltdata, float dt);
