#include "Khaibao.h"
#include "Run_Stop_Mtor.h"
#include "encoder.h"
#include "Calculate.h"
#include "Set_Position.h"
#include "Set_Home.h"

void setup() 
{
  pinMode(PUL_PIN_1, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(PUL_PIN_2, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(PUL_PIN_3, OUTPUT);
  pinMode(DIR_PIN_3, OUTPUT);
  pinMode(namcham, OUTPUT);

  pinMode(pinA_1, INPUT_PULLUP);
  pinMode(pinB_1, INPUT_PULLUP);
  pinMode(pinA_2, INPUT_PULLUP);
  pinMode(pinB_2, INPUT_PULLUP);
  pinMode(pinA_3, INPUT_PULLUP);
  pinMode(pinB_3, INPUT_PULLUP);

  // Bật ngắt encoder
  attachInterrupt(digitalPinToInterrupt(pinA_1), handleA_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_1), handleB_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinA_2), handleA_2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_2), handleB_2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinA_3), handleA_3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_3), handleB_3, CHANGE);

  pinMode(limit_1, INPUT_PULLUP);
  pinMode(limit_2, INPUT_PULLUP);
  pinMode(limit_3, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() 
{
  static unsigned long lastTrajectoryUpdate = 0;
  static unsigned long lastPrintUpdate = 0;
  
  // Xử lý serial input
  if (Serial.available()) 
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    Serial.println("Received: " + input); 
    
    if (input[0] == 's') 
    {
      StopMotor_1();
      StopMotor_2();
      StopMotor_3();
      trajectory_ready = false;
    }
    else if (input[0] == 'h')
    {
      SetHome();
    }
    else if (input[0] == 'u')
    {
      Serial.println("Hut vat");
      digitalWrite(namcham,1);
      inString = "";
    }
    else if (input[0] == 'd')
    {
      Serial.println("Nha vat");
      digitalWrite(namcham,0);
      inString = "";
    }
    else if (input.startsWith("P0:") && input.indexOf("Pf:") > 0 && input.indexOf("T:") > 0)
    {
      // Parse P0
      int p0_start = input.indexOf("P0:") + 3;
      int p0_end = input.indexOf(";");
      String p0_str = input.substring(p0_start, p0_end);
      int p0_comma1 = p0_str.indexOf(',');
      int p0_comma2 = p0_str.indexOf(',', p0_comma1 + 1);
      P0[0] = p0_str.substring(0, p0_comma1).toFloat();
      P0[1] = p0_str.substring(p0_comma1 + 1, p0_comma2).toFloat();
      P0[2] = p0_str.substring(p0_comma2 + 1).toFloat();

      // Parse Pf
      int pf_start = input.indexOf("Pf:") + 3;
      int pf_end = input.indexOf(";", p0_end + 1);
      String pf_str = input.substring(pf_start, pf_end);
      int pf_comma1 = pf_str.indexOf(',');
      int pf_comma2 = pf_str.indexOf(',', pf_comma1 + 1);
      Pf[0] = pf_str.substring(0, pf_comma1).toFloat();
      Pf[1] = pf_str.substring(pf_comma1 + 1, pf_comma2).toFloat();
      Pf[2] = pf_str.substring(pf_comma2 + 1).toFloat();

      int tf_start = input.indexOf("T:") + 2;
      String tf_str = input.substring(tf_start);
      tf = tf_str.toFloat();

      
      t0 = millis() / 1000.0; // Thời gian bắt đầu tính bằng giây
      t = 0;
      trajectory_ready = true;
      
      Serial.println("Trajectory planning started.");
      Serial.println("P0: " + String(P0[0]) + ", " + String(P0[1]) + ", " + String(P0[2]));
      Serial.println("Pf: " + String(Pf[0]) + ", " + String(Pf[1]) + ", " + String(Pf[2]));
      Serial.println("tf: " + String(tf) + " seconds");
    }
    else
    {
      // Xử lý các lệnh góc riêng lẻ nếu cần
      for (int x = 0; x < input.length(); x++) 
      {
        if ((input[x] == '-') || (input[x] == '.')) 
        {
          inString += (char)input[x];
        }
        if (isDigit(input[x])) 
        {
          inString += (char)input[x];
        }
      
        if (input[x] == 'A') 
        {
          deg1 = inString.toFloat();
          if (abs(deg1 - deg1_old) > 0.1) 
          {
            Degree_1(deg1, deg1_old);
            deg1_old = deg1;
          }
          inString = " ";
        } 
        else if (input[x] == 'B') 
        {
          deg2 = inString.toFloat();
          if (abs(deg2 - deg2_old) > 0.1) 
          {
            Degree_2(deg2, deg2_old);
            deg2_old = deg2;
          }
          inString = " ";
        }
        else if (input[x] == 'C') 
        {
          deg3 = inString.toFloat();
          if (abs(deg3 - deg3_old) > 0.1) 
          {
            Degree_3(deg3, deg3_old);
            deg3_old = deg3;
          }
          inString = " ";
        }
      }
    }
  }

  // Chạy các động cơ
  RunMotor_1();
  RunMotor_2();
  RunMotor_3();

  // Quy hoạch quỹ đạo
  if (trajectory_ready) {
    unsigned long now = millis();
    // Tính góc quay từ vị trí đầu và cuối
    float angles_start[3], angles_end[3];
    if (inverse_kinematic(P0[0], P0[1], P0[2], angles_start) && inverse_kinematic(Pf[0], Pf[1], Pf[2], angles_end)) 
    {

      float angle_deg_1 = abs(angles_end[0] - angles_start[0]);
      float angle_deg_2 = abs(angles_end[1] - angles_start[1]);
      float angle_deg_3 = abs(angles_end[2] - angles_start[2]);

      update_delay_run_spd(tf, angle_deg_1, angle_deg_2, angle_deg_3);
    }
    if (now - lastTrajectoryUpdate >= 10) { // Cập nhật mỗi 10ms (100Hz)
      lastTrajectoryUpdate = now;
      
      // Tính thời gian thực tế đã trôi qua
      float current_time = (millis() / 1000.0) - t0;
      t = current_time;
      
      float X_now, Y_now, Z_now;
      // trajectory_planning_2_point(t, P0, t0, Pf, tf, &X_now, &Y_now, &Z_now);
      trajectory_planning_2_point(t, P0, Pf, tf, &X_now, &Y_now, &Z_now);
      
      float angles[3];
      if (inverse_kinematic(X_now, Y_now, Z_now, angles)) {
        Degree_1(angles[0], deg1_old);
        deg1_old = angles[0];
        
        Degree_2(angles[1], deg2_old);
        deg2_old = angles[1];
        
        Degree_3(angles[2], deg3_old);
        deg3_old = angles[2];
      }

      if (t >= tf) {
        trajectory_ready = false;
        Serial.println("Trajectory finished.");
      }
    }
  }

  // // In thông tin encoder định kỳ
  // if (millis() - lastPrintUpdate >= 1000) {
  //   lastPrintUpdate = millis();
  //   Serial.print("Encoder 1: "); Serial.print(encoderPosition_1); Serial.print("  ");
  //   Serial.print("Encoder 2: "); Serial.print(encoderPosition_2); Serial.print("  ");
  //   Serial.print("Encoder 3: "); Serial.println(encoderPosition_3);
  // }
}