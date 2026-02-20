#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

// MUST be incremented when config struct is changed
// Epoch 1 0xDEAD0001 FEB-19-2026
// Epoch 2 0xDEAD0002 FEB-19-2026 Added switch to disable servo during burn
// Epoch 3 0xDEAD0003 FEB-19-2026 Added motor burn time
#define CFG_MAGIC 0xDEAD0003

typedef struct
{
    uint32_t magic;

    PIDCoeff_t pid_pitch;
    PIDCoeff_t pid_roll;
    PIDCoeff_t pid_yaw;

    float pid_i_max;

    float servo_center_us;
    float servo_limit_max_deg;
    float servo_us_per_deg;

    uint32_t motor_burn_time_ms;
    uint32_t parachute_charge_timeout_ms;

    uint32_t log_interval_ms;
    uint32_t log_flush_interval_ms;

    bool en_servo_in_burn;
    bool test_mode_en;

} EEPROMCfg_t;

extern EEPROMCfg_t config;

void config_init();
void config_save();
void config_set_defaults();
