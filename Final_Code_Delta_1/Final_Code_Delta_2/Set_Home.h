void SetHome()
{
  digitalWrite(DIR_PIN_1,0);
  digitalWrite(DIR_PIN_2,0);
  digitalWrite(DIR_PIN_3,0);
  delayMicroseconds(20);
  bool xHomed = false; // Cờ để kiểm tra trục X đã về gốc
  bool yHomed = false; // Cờ để kiểm tra trục Y đã về gốc
  bool zHomed = false; // Cờ để kiểm tra trục Z đã về gốc

  // Serial.print("Limit 1: "); Serial.println(digitalRead(limit_1));
  // Serial.print("Limit 2: "); Serial.println(digitalRead(limit_2));
  // Serial.print("Limit 3: "); Serial.println(digitalRead(limit_3));
  while (!xHomed || !yHomed || !zHomed)
  // while (!xHomed|| !zHomed)
  {
    // Set home cho motor 1
    if (!xHomed)
    {
      if (digitalRead(limit_1) == LOW)
      {
        delay(10); // chống dội
        if (digitalRead(limit_1) == LOW) // kiểm tra lại sau delay
        {
          StopMotor_1();
          delay(50);
          SetPosition_1();
          xHomed = true;
          Serial.println("Truc X da ve home.");
        }
      }
      else
      {
        digitalWrite(PUL_PIN_1, HIGH);
        delayMicroseconds(delay_home_spd); //80
        digitalWrite(PUL_PIN_1, LOW);
        delayMicroseconds(delay_home_spd);
      }
    }
    // // Set home cho motor 2
    if (!yHomed)
    {
      if (digitalRead(limit_2) == LOW)
      {
        delay(10); // chống dội
        if (digitalRead(limit_2) == LOW) // kiểm tra lại sau delay
        {
          StopMotor_2();
          delay(50);
          SetPosition_2();
          yHomed = true;
          Serial.println("Truc Y da ve home.");
        }
      }
      else
      {
        digitalWrite(PUL_PIN_2, HIGH);
        delayMicroseconds(delay_home_spd);
        digitalWrite(PUL_PIN_2, LOW);
        delayMicroseconds(delay_home_spd);
      }
    }
    // Set home cho motor 3
    if (!zHomed)
    {
      if (digitalRead(limit_3) == LOW)
      {
        delay(10); // chống dội
        if (digitalRead(limit_3) == LOW) // kiểm tra lại sau delay
        {
          StopMotor_3();
          delay(50);
          SetPosition_3();
          zHomed = true;
          Serial.println("Truc Z da ve home.");
        }
      }
      else
      {
        digitalWrite(PUL_PIN_3, HIGH);
        delayMicroseconds(delay_home_spd);
        digitalWrite(PUL_PIN_3, LOW);
        delayMicroseconds(delay_home_spd);
      }
    }
  }
  Degree_1(8.5, 0);
  Degree_2(12, 0);
  Degree_3(7, 0);

  // Cho phép loop() chạy tiếp để điều khiển motor
  motorRunning_1 = true;
  motorRunning_2 = true;
  motorRunning_3 = true;

  // Chờ đến khi tất cả đã chạy xong
  while (motorRunning_1 || motorRunning_2 || motorRunning_3) 
  {
    RunMotor_1();
    RunMotor_2();
    RunMotor_3();
  }
  delay(400);
  SetPosition_1();
  SetPosition_2();
  SetPosition_3();
}