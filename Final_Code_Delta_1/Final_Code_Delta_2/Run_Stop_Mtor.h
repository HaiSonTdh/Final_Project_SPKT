void StopMotor_1() 
{
  motorRunning_1 = false;
  digitalWrite(PUL_PIN_1, LOW);
  // endTime_1 = millis(); // Ghi lại thời gian kết thúc
  // unsigned long runDuration = endTime_1 - startTime_1;
  // Serial.print("Run time 1: ");
  // Serial.print(runDuration);
  // Serial.println(" ms");
}  
void StopMotor_2() 
{
  motorRunning_2 = false;
  digitalWrite(PUL_PIN_2, LOW);
  // endTime_2 = millis(); // Ghi lại thời gian kết thúc
  // unsigned long runDuration = endTime_2 - startTime_2;
  // Serial.print("Run time 2: ");
  // Serial.print(runDuration);
  // Serial.println(" ms");
}  
void StopMotor_3() 
{
  motorRunning_3 = false;
  digitalWrite(PUL_PIN_3, LOW);
  // endTime_3 = millis(); // Ghi lại thời gian kết thúc
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
    if (now - lastPulseTime_1 >= delay_run_spd) 
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
    if (now - lastPulseTime_2 >= delay_run_spd) 
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
    if (now - lastPulseTime_3 >= delay_run_spd) 
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

