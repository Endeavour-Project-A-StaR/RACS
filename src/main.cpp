#include <Arduino.h>
#include <Wire.h>
#include "types.h"
#include "imu.h"
#include "nav.h"
#include "log.h"
#include "actuators.h"
#include "persistent_config.h"
#include "comms.h"

FltStates_t state = STATE_DIAG; // Default startup to self test
FltData_t fltdata;              // Init shared flight data struct

uint32_t last_loop_time = 0;

void setup()
{

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(0); // ACM doesnt need baud rate

  Wire.begin();
  Wire.setClock(1000000);

  while (!Serial.available())
    delay(1);

  config_init();

  int ret = imu_init();
  if (!ret)
    while (1)
      delay(1);
  Serial.println("MSG: IMU INIT SUCCESS");

  servo_init(&fltdata);

  Serial.println("MSG: WILL TEST SERVO");

  servo_swing_test();

  Serial.println("MSG: SERVO RECENTERED");

  Serial.println("MSG: GYRO CAL IN 5 SEC");
  delay(5000);
  imu_cal_gyro(&fltdata);

  Serial.println("MSG: BAYES READY");

  nav_rst_integral();

  state = STATE_PREFLT;

  digitalWrite(LED_BUILTIN, HIGH);

  last_loop_time = micros();
}

void loop()
{
  uint32_t current_time = micros();
  float dt = (current_time - last_loop_time) / 1000000.0f;

  if (dt >= (1.0f / 1600.0f)) // 1600Hz
  {
    last_loop_time = current_time;

    if (imu_read(&fltdata))
    {
      switch (state)
      {

      case STATE_PREFLT:

        imu_calc_initial_att(&fltdata);

        break;

      case STATE_NAVLK:

        imu_calc_att(&fltdata, dt);

        if (fltdata.accel[0] > 20.0f)
        {
          state = STATE_BURN;
          Serial.println("MSG: LIFTOFF");
        }
        break;

      case STATE_BURN:

        imu_calc_att(&fltdata, dt);
        nav_update_pid(&fltdata, dt);

        if (fltdata.accel[0] < 0.0f)
        {
          state = STATE_COAST;
          Serial.println("MSG: BURNOUT");
        }

        break;

      case STATE_COAST:

        imu_calc_att(&fltdata, dt);
        nav_update_pid(&fltdata, dt);

        if (false)
        {
          state = STATE_RECVY;
          Serial.println("MSG: APOGEE REACHED");
        }

        break;

      case STATE_RECVY:
        break;

      case STATE_OVRD:

        imu_calc_att(&fltdata, dt);
        nav_update_pid(&fltdata, dt);

        break;

      default:

        break;
      }

      servo_write(&fltdata);

      // comms_send_telem(state, &fltdata);
    }
    else
    {
      Serial.println("MSG: IMU READ FAIL");
    }
  }

  comms_read_cmd(&state);
}
