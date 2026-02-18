#include "imu.h"
#include <math.h>
#include <Wire.h>
#include <ICM45686.h>

bool imu_init() 
{
return true; // dummy
}

bool imu_read(FltData_t* fltdata)
{
    return true;
}

void imu_gyro_cal(FltData_t* fltdata)
{
    return;
}

void imu_run_fusion(FltData_t* fltdata, float dt)
{
    return;
}