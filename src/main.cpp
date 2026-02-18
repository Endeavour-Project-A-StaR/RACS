#include <Arduino.h>
#include <Wire.h>
#include "types.h"
#include "imu.h"

FltStates_t state = STATE_DIAG; // Default startup to self test
FltData_t fltdata;              // Init shared flight data struct

uint32_t last_loop_time = 0;
uint32_t last_print_time = 0;

void setup()
{

  Serial.begin(0); // ACM doesnt need baud rate

  Wire.begin();
  Wire.setClock(1000000);

  while (!Serial.available())
    delay(1);

  Serial.println("------ Bayes V3_5 Test ------");

  int ret = imu_init();
  Serial.println(ret ? "IMU Init Success" : "IMU Init Failed");
  if (!ret)
    while (1)
      delay(1);

  Serial.println("Will calibrate gyro bias in 2 sec");
  delay(2000);
  imu_cal_gyro();
  Serial.println("Gyro calibration complete");

  state = STATE_PREFLT;

  last_loop_time = micros();
}

void loop()
{
  uint32_t current_time = micros();
  float dt = (current_time - last_loop_time) / 1000000.0f;

  if (dt >= 0.001f)
  {
    last_loop_time = current_time;
    imu_read(&fltdata);

    if (state == STATE_PREFLT)
    {
      imu_calc_initial_att(&fltdata);
    }
    else if (state == STATE_BURN)
    {
      imu_calc_att(&fltdata, dt);
    }

    if (millis() - last_print_time > 10)
    {
      last_print_time = millis();
      float pitch_deg = fltdata.pitch * (180.0f / 3.14159f);
      float roll_deg = fltdata.roll * (180.0f / 3.14159f);
      float yaw_deg = fltdata.yaw * (180.0f / 3.14159f);

      Serial.print(state == STATE_PREFLT ? "PREFLT | " : "FLT | ");
      Serial.print("P: ");
      Serial.print(pitch_deg, 1);
      Serial.print("\tR: ");
      Serial.print(roll_deg, 1);
      Serial.print("\tY: ");
      Serial.print(yaw_deg, 1);
      Serial.println();
    }
  }

  // 5. Read User Commands
  if (Serial.available())
  {
    char cmd = Serial.read();
    if (cmd == 'l')
    {
      state = STATE_BURN;
      Serial.println("\n*** LAUNCH DETECTED - SWITCHING TO GYRO ***\n");
    }
    else if (cmd == 'p')
    {
      state = STATE_PREFLT;
      Serial.println("\n*** PAD RESET - SWITCHING TO ACCEL ***\n");
    }
  }
}
