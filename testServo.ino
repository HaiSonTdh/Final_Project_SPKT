#include <math.h>

#define PUL_PIN_1 22   // Chân xung
#define DIR_PIN_1 24   // Chân hướng quay
#define PUL_PIN_2 26
#define DIR_PIN_2 28
#define PUL_PIN_3 30   
#define DIR_PIN_3 32

// #define PUL_PIN_1 265   // Chân xung
// #define DIR_PIN_1 345   // Chân hướng quay
// #define PUL_PIN_2 24
// #define DIR_PIN_2 33
// #define PUL_PIN_3 268   
// #define DIR_PIN_3 349

//Khai báo chân đọc encoder
#define pinA_1 2
#define pinB_1 3
#define pinA_2 18
#define pinB_2 19
#define pinA_3 20
#define pinB_3 21

#define READ_A_1 digitalRead(pinA_1)
#define READ_B_1 digitalRead(pinB_1)
#define READ_A_2 (PIND & (1 << PIND3))  // PIND2 = chân 18
#define READ_B_2 (PIND & (1 << PIND2))  // PIND3 = chân 19
#define READ_A_3 (PIND & (1 << PIND1))  // PIND là thanh ghi trạng thái (register) chứa dữ liệu của các chân digital 0 - 7 (tức là PORTD).
#define READ_B_3 (PIND & (1 << PIND0))  // Shift bit 1 sang trái 2 lần → 0b00000100
                                        // dùng phép AND để kiểm tra xem bit số 2 (chân 18) đang ở mức HIGH hay LOW 

const int limit_1 = 34;
const int limit_2 = 36;
const int limit_3 = 38;

volatile long encoderPosition_1 = 0;
volatile long encoderPosition_2 = 0;
volatile long encoderPosition_3 = 0;
volatile bool A_set = false;
volatile bool B_set = false;

float nPulse_1;
float deg1 = 0.0;
float deg1_old = 0.0;
float degree1 = 0.0;

float nPulse_2;
float deg2 = 0.0;
float deg2_old = 0.0;
float degree2 = 0.0;

float nPulse_3;
float deg3 = 0.0;
float deg3_old = 0.0;
float degree3 = 0.0;

// Variable global
String inString = "";

int motorRunning_1;
int motorRunning_2;
int motorRunning_3;

unsigned long startTime = 0;
unsigned long endTime = 0;

unsigned long lastPrintTime = 0;

void setup() 
{
  pinMode(PUL_PIN_1, OUTPUT);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(PUL_PIN_2, OUTPUT);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(PUL_PIN_3, OUTPUT);
  pinMode(DIR_PIN_3, OUTPUT);

  pinMode(pinA_1, INPUT_PULLUP);
  pinMode(pinB_1, INPUT_PULLUP);
  pinMode(pinA_2, INPUT_PULLUP);
  pinMode(pinB_2, INPUT_PULLUP);
  pinMode(pinA_3, INPUT_PULLUP);
  pinMode(pinB_3, INPUT_PULLUP);
  
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
// Nhập command: 10A20B30C --> motor1: 10 degree, motor2: 20 degree,...
void loop() 
{
  // int state = digitalRead(limit_1);
  // if(digitalRead(limit_2) == LOW)
  // {
  //   delay(50);
  //   if(digitalRead(limit_2) == LOW)
  //   {
  //     StopMotor_2();
  //     // Serial.print("touch");
  //     while(digitalRead(limit_2) == LOW)
  //     {delay(10);}
  //   }
  // }
  if (Serial.available()) 
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    // delayMicroseconds(1);
    Serial.println("Received: " + input); 
    if (input[0] == 's') 
    {
      StopMotor_1();
      StopMotor_2();
      StopMotor_3();
    }
    else if (input[0] == 'h')
    {
      SetHome();
    }
    // else
    // {
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
          // delay(200);
          deg1 = inString.toFloat();
          Degree_1(deg1, deg1_old);
          
          deg1_old = deg1;
          inString = " ";
        } 
        
        else if (input[x] == 'B') 
        {
          deg2 = inString.toFloat();
          // delay(500);
          Degree_2(deg2, deg2_old);
          
          deg2_old = deg2;
          inString = " ";
          // delay(500);
        }

        else if (input[x] == 'C') 
        {
          // delay(500); 
          deg3 = inString.toFloat();
          inString = " ";
          Degree_3(deg3, deg3_old);
          
          deg3_old = deg3;
          // delay(500);
        }

      }
    // }
  }
  float theta1 = deg1;
  float theta2 = deg2;
  float theta3 = deg3;
  RunMotor_1();
  RunMotor_2();
  RunMotor_3();

  if (millis() - lastPrintTime >= 2000) 
  { 
      Serial.print("Encoder 1: "); Serial.print(encoderPosition_1);  Serial.print("  ");
      Serial.print("Encoder 2: "); Serial.print(encoderPosition_2);  Serial.print("  ");
      Serial.print("Encoder 3: "); Serial.println(encoderPosition_3);  

      lastPrintTime = millis();
  }
  // Serial.println(encoderPosition_1);
  // delay(2000); 
  float Px, Py, Pz;

  bool success = ForwardKinematicUpdate(theta1, theta2, theta3, Px, Py, Pz);

  // if (success) {
  //   Serial.print("Px = ");
  //   Serial.print(Px);
  //   Serial.print(" mm, Py = ");
  //   Serial.print(Py);
  //   Serial.print(" mm, Pz = ");
  //   Serial.print(Pz);
  //   Serial.println(" mm");
  // } else {
  //   Serial.println("Không tìm được vị trí hợp lệ (z² < 0)");
  // }
  // delay(2000);
}

////////////////////////////////////////CALCULATE ANGLE////////////////////////////////////////
void Degree_1(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_1, 0);
    degree1 = abs(deg - deg_old);
    Serial.println("nghich"); 
  } 
  else
  {
    digitalWrite(DIR_PIN_1, 1);
    degree1 = abs(deg - deg_old);
    Serial.println("thuan"); 
  }
  nPulse_1 = degree1 * 10000 ;
  motorRunning_1 = true;

  startTime = millis();
}

void Degree_2(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_2, 0);
    degree2 = abs(deg - deg_old);
    Serial.println("nghich"); 
  } 
  else
  {
    digitalWrite(DIR_PIN_2, 1);
    degree2 = abs(deg - deg_old);
    Serial.println("thuan"); 
  }
  // nPulse_2 = degree2 * 8 * 10000 / 360;
  nPulse_2 = degree2 * 10000;
  motorRunning_2 = true;
  startTime = millis();
}

void Degree_3(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_3, 0);
    degree3 = abs(deg - deg_old);
    Serial.println("nghich"); 
  } 
  else
  {
    digitalWrite(DIR_PIN_3, 1);
    degree3 = abs(deg - deg_old);
    Serial.println("thuan"); 
  }
  // nPulse_3 = degree3 * 8 * 10000 / 360;
  nPulse_3 = degree3 * 10000;
  motorRunning_3 = true;
}

///////////////////////////////////////////RUN MOTOR//////////////////////////////////////////////
unsigned long lastPulseTime_1 = 0;
void RunMotor_1()
{
  if (motorRunning_1 && nPulse_1 > 0) 
  {
    unsigned long now = micros();
    if (now - lastPulseTime_1 >= 1) 
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
    if (now - lastPulseTime_2 >= 10) 
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
    if (now - lastPulseTime_3 >= 50) 
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

void StopMotor_1() 
{
  motorRunning_1 = false;
  digitalWrite(PUL_PIN_1, LOW);
  endTime = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime - startTime;
  Serial.print("Run time 1: ");
  Serial.print(runDuration);
  Serial.println(" ms");
}  
void StopMotor_2() 
{
  motorRunning_2 = false;
  digitalWrite(PUL_PIN_2, LOW);
  endTime = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime - startTime;
  Serial.print("Run time 2: ");
  Serial.print(runDuration);
  Serial.println(" ms");
}  
void StopMotor_3() 
{
  motorRunning_3 = false;
  digitalWrite(PUL_PIN_3, LOW);
  endTime = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime - startTime;
  Serial.print("Run time 3: ");
  Serial.print(runDuration);
  Serial.println(" ms");
}  

////////////////////////////////////////////SET HOME//////////////////////////////////////////////////
void SetHome()
{
  digitalWrite(DIR_PIN_1,0);
  delayMicroseconds(20);
  digitalWrite(DIR_PIN_2,0);
  delayMicroseconds(20);
  digitalWrite(DIR_PIN_3,0);
  delayMicroseconds(20);
  bool xHomed = false; // Cờ để kiểm tra trục X đã về gốc
  bool yHomed = false; // Cờ để kiểm tra trục Y đã về gốc
  bool zHomed = false; // Cờ để kiểm tra trục Z đã về gốc
  // while (!xHomed || !yHomed || !zHomed)
  // while (!xHomed || !yHomed || !zHomed)
  // {
  //   // Set home cho motor 1
  //   if (!xHomed)
  //   {
  //     if (digitalRead(limit_1) == HIGH)
  //     {
  //       digitalWrite(PUL_PIN_1,HIGH);
  //       delayMicroseconds(80);
  //       digitalWrite(PUL_PIN_1,LOW);
  //       delayMicroseconds(80);
  //     }
  //     else
  //     {
  //       StopMotor_1();
  //       delay(50);
  //       SetPosition_1();
  //       xHomed = true;
  //     }
  //   }
  //   // Set home cho motor 2
  //   if (!yHomed)
  //   {
  //     if (digitalRead(limit_2) == HIGH)
  //     {
  //       digitalWrite(PUL_PIN_2,HIGH);
  //       delayMicroseconds(80);
  //       digitalWrite(PUL_PIN_2,LOW);
  //       delayMicroseconds(80);
  //     }
  //     else
  //     {
  //       StopMotor_2();
  //       delay(50);
  //       SetPosition_2();
  //       Serial.print("Encoder sau reset: ");
  //       Serial.println(encoderPosition_2);
  //       yHomed = true;
  //     }
  //   }
  //   // Set home cho motor 3
  //   if (!zHomed)
  //   {
  //     if (digitalRead(limit_3) == HIGH)
  //     {
  //       digitalWrite(PUL_PIN_3,HIGH);
  //       delayMicroseconds(80);
  //       digitalWrite(PUL_PIN_3,LOW);
  //       delayMicroseconds(80);
  //     }
  //     else
  //     {
  //       StopMotor_3();
  //       delay(50);
  //       SetPosition_3();
  //       Serial.print("Encoder sau reset: ");
  //       Serial.println(encoderPosition_3);
  //       zHomed = true;
  //     }
  //   }
  // }
  Serial.print("Limit 1: "); Serial.println(digitalRead(limit_1));
  Serial.print("Limit 2: "); Serial.println(digitalRead(limit_2));
  Serial.print("Limit 3: "); Serial.println(digitalRead(limit_3));
  while (!xHomed || !yHomed || !zHomed)
  {
    // Set home cho motor 1
    if (!xHomed)
    {
      if (digitalRead(limit_1) == LOW)
      {
        StopMotor_1();
        delay(50);
        SetPosition_1();
        xHomed = true;
        Serial.println("Truc X da ve home.");
      }
      else
      {
        digitalWrite(PUL_PIN_1, HIGH);
        delayMicroseconds(80);
        digitalWrite(PUL_PIN_1, LOW);
        delayMicroseconds(80);
      }
    }
    // Set home cho motor 2
    if (!yHomed)
    {
      if (digitalRead(limit_2) == LOW)
      {
        StopMotor_2();
        delay(50);
        SetPosition_2();
        yHomed = true;
        Serial.println("Truc Y da ve home.");
      }
      else
      {
        digitalWrite(PUL_PIN_2, HIGH);
        delayMicroseconds(80);
        digitalWrite(PUL_PIN_2, LOW);
        delayMicroseconds(80);
      }
    }
    // Set home cho motor 3
    if (!zHomed)
    {
      if (digitalRead(limit_3) == LOW)
      {
        StopMotor_3();
        delay(50);
        SetPosition_3();
        zHomed = true;
        Serial.println("Truc Z da ve home.");
      }
      else
      {
        digitalWrite(PUL_PIN_3, HIGH);
        delayMicroseconds(80);
        digitalWrite(PUL_PIN_3, LOW);
        delayMicroseconds(80);
      }
    }
  }
  Serial.println("Da thoat khoi SetHome.");
}
void SetPosition_1()
{
  detachInterrupt(digitalPinToInterrupt(pinA_1)); 
  detachInterrupt(digitalPinToInterrupt(pinB_1));
  deg1 = 0.0;
  deg1_old = 0.0;
  degree1 = 0.0;
  
  encoderPosition_1 = 0;
  nPulse_1 = 0; 
  delayMicroseconds(50);
  attachInterrupt(digitalPinToInterrupt(pinA_1), handleA_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_1), handleB_1, CHANGE);
}
void SetPosition_2()
{
  deg2 = 0.0;
  deg2_old = 0.0;
  degree2 = 0.0;

  detachInterrupt(digitalPinToInterrupt(pinA_2)); 
  detachInterrupt(digitalPinToInterrupt(pinB_2));
  encoderPosition_2 = 0;
  nPulse_2 = 0; 
  attachInterrupt(digitalPinToInterrupt(pinA_2), handleA_2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_2), handleB_2, CHANGE);
}
void SetPosition_3()
{
  deg3 = 0.0;
  deg3_old = 0.0;
  degree3 = 0.0;
  detachInterrupt(digitalPinToInterrupt(pinA_3)); 
  detachInterrupt(digitalPinToInterrupt(pinB_3));
  delayMicroseconds(100); // hoặc nhỏ hơn nếu cần

  encoderPosition_3 = 0;
  nPulse_3 = 0;

  attachInterrupt(digitalPinToInterrupt(pinA_3), handleA_3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_3), handleB_3, CHANGE);
}

///////////////////////////////////////////RECEIVE ENCODER VALUE/////////////////////////////////////
// Hàm đếm xung encoder khi có sự thay đổi trạng thái
void handleA_1() {
  if (READ_A_1) {
    if (READ_B_1) encoderPosition_1--;
    else encoderPosition_1++;
  } else {
    if (READ_B_1) encoderPosition_1++;
    else encoderPosition_1--;
  }
}
void handleB_1() {
  if (READ_B_1) {
    if (READ_A_1) encoderPosition_1++;
    else encoderPosition_1--;
  } else {
    if (READ_A_1) encoderPosition_1--;
    else encoderPosition_1++;
  }
}

void handleA_2() {
  if (READ_A_2) {
    if (READ_B_2) encoderPosition_2--;
    else encoderPosition_2++;
  } else {
    if (READ_B_2) encoderPosition_2++;
    else encoderPosition_2--;
  }
}
void handleB_2() {
  if (READ_B_2) {
    if (READ_A_2) encoderPosition_2++;
    else encoderPosition_2--;
  } else {
    if (READ_A_2) encoderPosition_2--;
    else encoderPosition_2++;
  }
}

void handleA_3() {
  if (READ_A_3) {
    if (READ_B_3) encoderPosition_3--;
    else encoderPosition_3++;
  } else {
    if (READ_B_3) encoderPosition_3++;
    else encoderPosition_3--;
  }
}
void handleB_3() {
  if (READ_B_3) {
    if (READ_A_3) encoderPosition_3++;
    else encoderPosition_3--;
  } else {
    if (READ_A_3) encoderPosition_3--;
    else encoderPosition_3++;
  }
}
/////////////////////////////////////////////KINEMATIC/////////////////////////////////////

// Cấu hình hình học robot
const float R_base = 60.0;        // mm
const float R_platform = 42.62;   // mm
const float rf = 350.0;           // mm
const float re = 150.0;           // mm

bool ForwardKinematicUpdate(float theta1_deg, float theta2_deg, float theta3_deg,
                            float &Px, float &Py, float &Pz) 
{
    float r = R_base - R_platform;

    // Chuyển độ sang radian
    float theta[3] = {
        theta1_deg * PI / 180.0,
        theta2_deg * PI / 180.0,
        theta3_deg * PI / 180.0
    };

    float alpha[3] = {0.0, 2.0 * PI / 3.0, 4.0 * PI / 3.0};

    // Tọa độ 3 khớp động
    float P[3][3];
    for (int i = 0; i < 3; i++) {
        float angle = theta[i];
        float alpha_i = alpha[i];
        float x = (r + re * cos(angle)) * cos(alpha_i);
        float y = (r + re * cos(angle)) * sin(alpha_i);
        float z = -re * sin(angle);  // Z hướng xuống

        P[i][0] = x;
        P[i][1] = y;
        P[i][2] = z;
    }

    // Vector v12 = P2 - P1, v13 = P3 - P1
    float v12[3], v13[3];
    for (int i = 0; i < 3; i++) {
        v12[i] = P[1][i] - P[0][i];
        v13[i] = P[2][i] - P[0][i];
    }

    // Tính ex = v12 / |v12|
    float norm_v12 = sqrt(v12[0]*v12[0] + v12[1]*v12[1] + v12[2]*v12[2]);
    float ex[3];
    for (int i = 0; i < 3; i++) ex[i] = v12[i] / norm_v12;

    // Tính a = dot(ex, v13)
    float a = 0;
    for (int i = 0; i < 3; i++) a += ex[i] * v13[i];

    // Tính ey = (v13 - a*ex) / |...|
    float ey_tmp[3];
    for (int i = 0; i < 3; i++) ey_tmp[i] = v13[i] - a * ex[i];

    float norm_ey = sqrt(ey_tmp[0]*ey_tmp[0] + ey_tmp[1]*ey_tmp[1] + ey_tmp[2]*ey_tmp[2]);
    float ey[3];
    for (int i = 0; i < 3; i++) ey[i] = ey_tmp[i] / norm_ey;

    // Tính ez = ex x ey
    float ez[3];
    ez[0] = ex[1]*ey[2] - ex[2]*ey[1];
    ez[1] = ex[2]*ey[0] - ex[0]*ey[2];
    ez[2] = ex[0]*ey[1] - ex[1]*ey[0];

    // b = dot(ey, v13)
    float b = 0;
    for (int i = 0; i < 3; i++) b += ey[i] * v13[i];

    float d = norm_v12;
    float x = d / 2.0;
    float y = ((a*a + b*b) / (2.0 * b)) - (a * x / b);
    float z_sq = rf*rf - x*x - y*y;

    if (z_sq < 0) {
        // Không tồn tại vị trí hợp lệ
        return false;
    }

    float z = sqrt(z_sq);

    // Vị trí end-effector = P1 + x*ex + y*ey - z*ez
    Px = P[0][0] + x * ex[0] + y * ey[0] - z * ez[0];
    Py = P[0][1] + x * ex[1] + y * ey[1] - z * ez[1];
    Pz = P[0][2] + x * ex[2] + y * ey[2] - z * ez[2];

    return true;
}

