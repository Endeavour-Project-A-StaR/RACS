#include "imu.h"
#include <math.h>
#include <Wire.h>
#include <ICM45686.h>

static const uint16_t ACCEL_FSR_G = 16;
static const uint16_t GYRO_FSR_DPS = 2000;
static const uint16_t ODR_HZ = 1600;

static const float G_MS2 = 9.80665f;
static const float DEG_2_RAD = 3.14159265f / 180.0f;

static const float ACCEL_SCALE = (float)ACCEL_FSR_G / 32768.0f * G_MS2;
static const float GYRO_SCALE = (float)GYRO_FSR_DPS / 32768.0f * DEG_2_RAD;

static const int IMU_CAL_SMPS = 200;

static ICM456xx IMU(Wire, 0);

bool imu_init()
{
    int ret = IMU.begin();
    if (ret != 0)
        return false;

    ret = IMU.startAccel(ODR_HZ, ACCEL_FSR_G);
    if (ret != 0)
        return false;

    ret = IMU.startGyro(ODR_HZ, GYRO_FSR_DPS);
    if (ret != 0)
        return false;

    return true;
}

bool imu_read(FltData_t *fltdata)
{
    inv_imu_sensor_data_t imu_data;

    int ret = IMU.getDataFromRegisters(imu_data);
    if (ret != 0)
        return false;

    fltdata->accel[0] = (float)imu_data.accel_data[0] * ACCEL_SCALE;
    fltdata->accel[1] = (float)imu_data.accel_data[1] * ACCEL_SCALE;
    fltdata->accel[2] = (float)imu_data.accel_data[2] * ACCEL_SCALE;

    fltdata->gyro[0] = (float)imu_data.gyro_data[0] * GYRO_SCALE;
    fltdata->gyro[1] = (float)imu_data.gyro_data[1] * GYRO_SCALE;
    fltdata->gyro[2] = (float)imu_data.gyro_data[2] * GYRO_SCALE;

    return true;
}

void imu_att_align(FltData_t *fltdata)
{

}

void imu_run_fusion(FltData_t *fltdata, float dt)
{
    return;
}