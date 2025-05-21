// Hàm tính delay_run_spd dựa trên tf và góc quay (đơn vị deg)
void update_delay_run_spd(float tf, float angle_deg_1, float angle_deg_2, float angle_deg_3)
{
  if (angle_deg_1 == 0) angle_deg_1 = 0.0001; // tránh chia cho 0
  if (angle_deg_2 == 0) angle_deg_2 = 0.0001;
  if (angle_deg_3 == 0) angle_deg_3 = 0.0001;
  const float lpulse_deg = 0.225; // 1 xung quay được 0.225 độ
  float delay_seconds_1 = tf * lpulse_deg / angle_deg_1;  // thời gian delay giữa các xung (giây)
  float delay_seconds_2 = tf * lpulse_deg / angle_deg_2;  // thời gian delay giữa các xung (giây)
  float delay_seconds_3 = tf * lpulse_deg / angle_deg_3;  // thời gian delay giữa các xung (giây)
  delay_run_spd_1 = (unsigned long)(delay_seconds_1 * 1e6); // chuyển sang microseconds
  delay_run_spd_2 = (unsigned long)(delay_seconds_2 * 1e6); // chuyển sang microseconds
  delay_run_spd_3 = (unsigned long)(delay_seconds_3 * 1e6); // chuyển sang microseconds
    // Debug
  Serial.print("Updated delays (µs) - M1: "); Serial.print(delay_run_spd_1);
  Serial.print(" | M2: "); Serial.print(delay_run_spd_2);
  Serial.print(" | M3: "); Serial.println(delay_run_spd_3);
}
void StopMotor_1() {
  motorRunning_1 = false;
  digitalWrite(PUL_PIN_1, LOW);
  // endTime_1 = millis();
  // unsigned long runDuration = endTime_1 - startTime_1;
  // Serial.print("Run time 1: ");
  // Serial.print(runDuration);
  // Serial.println(" ms");
}

void StopMotor_2() {
  motorRunning_2 = false;
  // digitalWrite(PUL_PIN_2, LOW);
  // endTime_2 = millis();
  // unsigned long runDuration = endTime_2 - startTime_2;
  // Serial.print("Run time 2: ");
  // Serial.print(runDuration);
  // Serial.println(" ms");
}

void StopMotor_3() {
  motorRunning_3 = false;
  digitalWrite(PUL_PIN_3, LOW);
  // endTime_3 = millis();
  // unsigned long runDuration = endTime_3 - startTime_3;
  // Serial.print("Run time 3: ");
  // Serial.print(runDuration);
  // Serial.println(" ms");
}

unsigned long lastPulseTime_1 = 0;
void RunMotor_1()
{
  if (motorRunning_1 && nPulse_1 > 0) 
  {
    unsigned long now = micros();
    if (now - lastPulseTime_1 >= delay_run_spd_1) 
    { // mỗi 60us (30us high + 30us low)
      digitalWrite(PUL_PIN_1, !digitalRead(PUL_PIN_1)); // Toggle xung
      lastPulseTime_1 = now;

      if (digitalRead(PUL_PIN_1) == LOW) 
      { // mỗi chu kỳ LOW xong giảm 1 xung
        nPulse_1--;
        if (nPulse_1 <= 0) 
        {
          StopMotor_1();
        }
      }
    }
  }
}

unsigned long lastPulseTime_2 = 0;
void RunMotor_2()
{
  if (motorRunning_2 && nPulse_2 > 0) 
  {
    unsigned long now = micros();
    if (now - lastPulseTime_2 >= delay_run_spd_2) 
    { // mỗi 60us (30us high + 30us low)
      digitalWrite(PUL_PIN_2, !digitalRead(PUL_PIN_2)); // Toggle xung
      lastPulseTime_2 = now;

      if (digitalRead(PUL_PIN_2) == LOW) 
      { // mỗi chu kỳ LOW xong giảm 1 xung
        nPulse_2--;
        if (nPulse_2 <= 0) 
        {
          StopMotor_2();
        }
      }
    }
  }
}

unsigned long lastPulseTime_3 = 0;
void RunMotor_3()
{
  if (motorRunning_3 && nPulse_3 > 0) 
  {
    unsigned long now = micros();
    if (now - lastPulseTime_3 >= delay_run_spd_3) 
    { // mỗi 60us (30us high + 30us low)
      digitalWrite(PUL_PIN_3, !digitalRead(PUL_PIN_3)); // Toggle xung
      lastPulseTime_3 = now;

      if (digitalRead(PUL_PIN_3) == LOW) 
      { // mỗi chu kỳ LOW xong giảm 1 xung
        nPulse_3--;
        if (nPulse_3 <= 0) 
        {
          StopMotor_3();
        }
      }
    }
  }
}
