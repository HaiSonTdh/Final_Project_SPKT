#include <TimerOne.h>
#include <TimerThree.h>
// #include <TimerFive.h>
#include <math.h>
#include "Khaibao.h"
#include "Run_Stop_Mtor.h"
#include "encoder.h"
#include "Calculate.h"
#include "Set_Position.h"
#include "Set_Home.h"
#include "trajectory.h"

unsigned long lastTrajectoryTime = 0;      // Lưu thời điểm lần chạy gần nhất
unsigned long stepInterval_ms = 50; 
void setup() 
{
  pinMode(PUL_PIN_1, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(PUL_PIN_2, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(PUL_PIN_3, OUTPUT);
  pinMode(DIR_PIN_3, OUTPUT);
  pinMode(namcham, OUTPUT);
  pinMode(conveyor, OUTPUT);

  pinMode(pinA_1, INPUT_PULLUP);
  pinMode(pinB_1, INPUT_PULLUP);
  pinMode(pinA_2, INPUT_PULLUP);
  pinMode(pinB_2, INPUT_PULLUP);
  pinMode(pinA_3, INPUT_PULLUP);
  pinMode(pinB_3, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pinA_1), handleA_1, CHANGE); //digitalPinToInterrupt(pinA_1) --> chuyển đổi chân pinA_1 thành chân ngắt
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
  if (Serial.available()) 
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    // Serial.println("Received: " + input); 
    // if (parseTrajectoryCommand(input)) 
    // {
    //   trajectory_active = true;
    //   t_start = millis();
    // }
    if (input.indexOf('A') != -1 || input.indexOf('B') != -1 || input.indexOf('C') != -1) 
    // tìm kí tự A,B or C trong chuỗi input
    {
      handleAngleCommand(input);  // Xử lý điều khiển từng góc
    }
    // Nếu có dấu | → là quy hoạch quỹ đạo
    else if (input.indexOf('|') != -1) 
    {
      if (parseTrajectoryCommand(input)) 
      {
        trajectory_active = true;
        t_start = millis();
      }
    }
    else if (input[0] == 's') 
    {
      StopMotor_1();
      StopMotor_2();
      StopMotor_3();
    }
    else if (input[0] == 'h')
    {
      SetHome();
    }
    else if (input[0] == 'u')
    {
      digitalWrite(namcham,1);
      inString = "";
    }
    else if (input[0] == 'd')
    {
      digitalWrite(namcham,0);
      inString = "";
    }
  }
  // if (trajectory_active) 
  // {
  //   float t_now = (millis() - t_start) / 1000.0;
  //   if (t_now <= tf) 
  //   {
  //     float x, y, z;
  //     ComputeTrajectory(t_now, P0, Pf, tf, x, y, z); // bậc 5
  //     float thetas[3];
  //     if (inverse_kinematic(x, y, z, thetas))
  //     {
  //       runToAngle(1, thetas[0]);
  //       runToAngle(2, thetas[1]);
  //       runToAngle(3, thetas[2]);
  //     }
  //     else 
  //     {
  //       Serial.println("Lỗi IK: Không tính được góc.");
  //       trajectory_active = false;
  //       StopMotor_1(); StopMotor_2(); StopMotor_3();
  //     }
  //   } 
  //   else 
  //   {
  //     trajectory_active = false;
  //     StopMotor_1(); StopMotor_2(); StopMotor_3();
  //   }
  // }
  if (trajectory_active) 
  {
    unsigned long currentMillis = millis();
    if (currentMillis - lastTrajectoryTime >= stepInterval_ms) 
    {
      lastTrajectoryTime = currentMillis;

      float t_now = (currentMillis - t_start) / 1000.0;
      if (t_now <= tf) 
      {
        float x, y, z;
        ComputeTrajectory(t_now, P0, Pf, tf, x, y, z);

        float thetas[3];
        if (inverse_kinematic(x, y, z, thetas)) 
        {
          runToAngle(1, thetas[0]);
          runToAngle(2, thetas[1]);
          runToAngle(3, thetas[2]);
        }
        else 
        {
          Serial.println("Lỗi IK: Không tính được góc.");
          trajectory_active = false;
          StopMotor_1(); StopMotor_2(); StopMotor_3();
        }

        Serial.print("t_now = "); Serial.println(t_now, 3);
      } 
      else 
      {
        trajectory_active = false;
        StopMotor_1(); StopMotor_2(); StopMotor_3();
      }
    }
  }
  
  encoderRawNow_1 = encoderPosition_1;
  delta_1 = encoderRawNow_1 - encoderRawLast_1;
  encoderRawLast_1 = encoderRawNow_1;
  
  if (lastDir_1_encoder == 1)
    encoderCalibrated_1 += delta_1 * encoderGainThuan;
  else
    encoderCalibrated_1 += delta_1 * encoderGainNghich;

  // long rawNow_2 = encoderRaw_2;
  // long delta_2 = rawNow_2 - lastEncoderRaw_2;
  // lastEncoderRaw_2 = rawNow_2;

  // if (lastDir_2_encoder == -1)  // quay ngược
  //     encoderPosition_2 += delta_2 * 2;
  // else
  //     encoderPosition_2 += delta_2;

  long rawNow_3 = encoderRaw_3;
  delta_3 = rawNow_3 - lastEncoderRaw_3;
  lastEncoderRaw_3 = rawNow_3;

  if (lastDir_3_encoder == -1)  // quay ngược
      encoderPosition_3 += delta_3 * 2;
  else
      encoderPosition_3 += delta_3;

  if (millis() - lastPrintTime >= 2000) 
  { 
      Serial.print("Encoder 1: "); Serial.print((long)encoderCalibrated_1);  Serial.print("  ");
      Serial.print("Encoder 2: "); Serial.print(encoderPosition_2);  Serial.print("  ");
      Serial.print("Encoder 3: "); Serial.println(encoderPosition_3); 
      // (long)encoderCalibrated_1 
      lastPrintTime = millis();
  }
  RunMotor_1();
  RunMotor_2();
  RunMotor_3();
}
void handleAngleCommand(String input)
{
  String inString = "";
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