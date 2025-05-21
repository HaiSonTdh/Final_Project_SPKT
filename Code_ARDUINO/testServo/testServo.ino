#include <math.h>
#define PUL_PIN_1 22   // Chân xung
#define DIR_PIN_1 24   // Chân hướng quay
#define PUL_PIN_2 26
#define DIR_PIN_2 28
#define PUL_PIN_3 30   
#define DIR_PIN_3 32

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
                                        // -> đọc nhanh trạng thái chân 18,19 bằng cách truy xuất trực tiếp thanh ghi và kiểm tra bit thứ 2,3
const int limit_1 = 34;
const int limit_2 = 36;
const int limit_3 = 38;
const int namcham = 40;

volatile long encoderPosition_1 = 0;
volatile long encoderPosition_2 = 0;
volatile long encoderPosition_3 = 0;
volatile float encoderCalibrated_1 = 0.0;
float encoderGainThuan = 10000.0 / 5547.0;   // ≈ 1.817
float encoderGainNghich = 10000.0 / 4982.0;  // ≈ 1.602

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

unsigned long startTime_1,startTime_2,startTime_3;
unsigned long endTime_1,endTime_2,endTime_3;
unsigned long lastPrintTime = 0;

unsigned long delay_run_spd = 3000;
unsigned long delay_home_spd = 4000;
unsigned long pulperrev = 200;

volatile int lastDir_1_encoder = 1;  // 1 = thuận, -1 = nghịch
volatile int lastDir_2_encoder = 1;  // 1 = thuận, -1 = nghịch
volatile int lastDir_3_encoder = 1; // 1 = thuận, -1 = nghịch
long encoderRawLast_1 = 0;
long encoderRawNow_1 = 0;
long delta_1 = 0;
volatile long encoderRaw_2 = 0;
long lastEncoderRaw_2 = 0;    
long delta_3 = 0;
volatile long encoderRaw_3 = 0;      // Encoder chưa hiệu chỉnh (raw, đọc trong ISR)
long lastEncoderRaw_3 = 0;           // Dùng để tính delta ngoài ISR

float P0[3], Pf[3];
float tf = 5.0;
unsigned long t_start;
bool trajectory_active = false;
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
// Nhập command: 10A20B30C --> motor1: 10 degree, motor2: 20 degree,...
void loop() 
{
  if (Serial.available()) 
  {
    String input = Serial.readStringUntil('\n');
    input.trim();
    Serial.println("Received: " + input); 
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
  }
  if (trajectory_active) 
  {
    float t_now = (millis() - t_start) / 1000.0;
    if (t_now <= tf) 
    {
      float x, y, z;
      ComputeTrajectory(t_now, P0, Pf, tf, x, y, z); // bậc 5
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
    } 
    else 
    {
      trajectory_active = false;
      StopMotor_1(); StopMotor_2(); StopMotor_3();
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

void runToAngle(int motorID, float angle) 
{
  switch (motorID) {
    case 1: Degree_1(angle, deg1_old);  break;
    case 2: Degree_2(angle, deg2_old);  break;
    case 3: Degree_3(angle, deg3_old);  break;
  }
}
// indexOf: tính tổng các kí tự trc khi thấy kí tự |
// substring: cắt từ vị trí chỉ định tới vị trí yêu cầu str.substr(start, length);
bool parseTrajectoryCommand(String input) 
{
  input.trim();  // Xóa khoảng trắng hoặc ký tự xuống dòng

  int sep1 = input.indexOf('|');
  int sep2 = input.lastIndexOf('|');

  if (sep1 == -1 || sep2 == -1 || sep1 == sep2) {
    Serial.println("Lỗi định dạng chuỗi.");
    return false;
  }

  String part_P0 = input.substring(0, sep1);
  String part_Pf = input.substring(sep1 + 1, sep2);
  String part_tf = input.substring(sep2 + 1);

  int c1 = part_P0.indexOf(',');
  int c2 = part_P0.lastIndexOf(',');
  if (c1 == -1 || c2 == -1 || c1 == c2) return false;
  P0[0] = part_P0.substring(0, c1).toFloat();
  P0[1] = part_P0.substring(c1 + 1, c2).toFloat();
  P0[2] = part_P0.substring(c2 + 1).toFloat();

  c1 = part_Pf.indexOf(',');
  c2 = part_Pf.lastIndexOf(',');
  if (c1 == -1 || c2 == -1 || c1 == c2) return false;
  Pf[0] = part_Pf.substring(0, c1).toFloat();
  Pf[1] = part_Pf.substring(c1 + 1, c2).toFloat();
  Pf[2] = part_Pf.substring(c2 + 1).toFloat();

  // -------- Tính tf động dựa vào góc quay và xung --------
  float theta_start[3], theta_end[3];
  if (!inverse_kinematic(P0[0], P0[1], P0[2], theta_start)) {
    Serial.println("Lỗi IK tại P0");
    return false;
  }
  if (!inverse_kinematic(Pf[0], Pf[1], Pf[2], theta_end)) {
    Serial.println("Lỗi IK tại Pf");
    return false;
  }

  float d1 = abs(theta_end[0] - theta_start[0]);
  float d2 = abs(theta_end[1] - theta_start[1]);
  float d3 = abs(theta_end[2] - theta_start[2]);

  float pulses_1 = d1 * pulperrev * 8 / 360.0;
  float pulses_2 = d2 * pulperrev * 8 / 360.0;
  float pulses_3 = d3 * pulperrev * 8 / 360.0;

  float t1 = pulses_1 * delay_run_spd / 1000000.0;
  float t2 = pulses_2 * delay_run_spd / 1000000.0;
  float t3 = pulses_3 * delay_run_spd / 1000000.0;

  tf = max(t1, max(t2, t3));  // Gán lại tf

  // In kiểm tra
  Serial.print("Tự tính tf = "); Serial.println(tf, 4);
  Serial.print("P0 = "); Serial.print(P0[0]); Serial.print(", "); Serial.print(P0[1]); Serial.print(", "); Serial.println(P0[2]);
  Serial.print("Pf = "); Serial.print(Pf[0]); Serial.print(", "); Serial.print(Pf[1]); Serial.print(", "); Serial.println(Pf[2]);

  return true; // CHUYỂN RETURN XUỐNG CUỐI
}

// *P0: P0 là con trỏ, lấy giá trị tại địa chỉ mà P0 trỏ đến
// &x: lấy địa chỉ ô nhớ của x
void ComputeTrajectory(float t, float *P0, float *Pf, float tf, float &x, float &y, float &z) 
{
  auto compute_axis = [&](float p0, float pf, float t) -> float {
    float delta = pf - p0;
    float a0 = p0;
    float a1 = 0;
    float a2 = 0;
    float a3 = 10 * delta / (tf*tf*tf);
    float a4 = -15 * delta / (tf*tf*tf*tf);
    float a5 = 6 * delta / (tf*tf*tf*tf*tf);
    return a0 + a1 * t + a2 * t*t + a3 * t*t*t + a4 * t*t*t*t + a5 * t*t*t*t*t;
  };

  x = compute_axis(P0[0], Pf[0], t);
  y = compute_axis(P0[1], Pf[1], t);
  z = compute_axis(P0[2], Pf[2], t);
}

bool inverse_kinematic(float X_ee, float Y_ee, float Z_ee, float *J) // J là mảng
{
  float R_Base = 60.0;
  float R_platform = 42.62;
  float r = R_Base - R_platform;

  float re = 150.0;
  float rf = 350.0;
  float threshold = 0.001;

  float alpha_deg[3] = {0, 120, 240};

  for (int i = 0; i < 3; i++) {
    float alpha = radians(alpha_deg[i]);
    float cos_alpha = cos(alpha);
    float sin_alpha = sin(alpha);

    float A = -2.0 * re * (-r + X_ee * cos_alpha + Y_ee * sin_alpha);
    float B = -2.0 * re * Z_ee;
    float C = (X_ee * X_ee + Y_ee * Y_ee + Z_ee * Z_ee + r * r + re * re - rf * rf
              - 2 * r * (X_ee * cos_alpha + Y_ee * sin_alpha));

    float denominator = sqrt(A * A + B * B); // fabs: trả về giá trị tuyệt đối
    if (denominator == 0 || fabs(C / denominator) > 1.0) {
      Serial.println("Lỗi: Không thể tính góc do acos out of range");
      return false;
    }

    float theta1 = atan2(B, A) + acos(-C / denominator);
    float theta2 = atan2(B, A) - acos(-C / denominator);

    float theta;
    if (theta1 > theta2) {theta = theta2;}
    else {theta = theta1;}
    if (fabs(theta) < threshold) theta = 0;

    J[i] = -degrees(theta);  // Lưu vào mảng đầu ra
  }

  return true;
}
///////////////////////////////////////////RECEIVE ENCODER VALUE/////////////////////////////////////
// Hàm đếm xung encoder khi có sự thay đổi trạng thái
// TTL: Transistor-Transistor Logic,  hoạt động trong khoảng 0-5V
void handleA_1() {
  int delta = 0;
  if (READ_A_1) {
    if (READ_B_1) delta = -1;
    else delta = +1;
  } else {
    if (READ_B_1) delta = +1;
    else delta = -1;
  }
  encoderPosition_1 += delta;
}
void handleB_1() {
  int delta = 0;
  if (READ_B_1) {
    if (READ_A_1) delta = +1;
    else delta = -1;
  } else {
    if (READ_A_1) delta = -1;
    else delta = +1;
  }
  encoderPosition_1 += delta;
}
void handleA_2() {
  if (READ_A_2) {
    if (READ_B_2) encoderPosition_2 --;
    else encoderPosition_2 ++;
  } else {
    if (READ_B_2) encoderPosition_2 ++;
    else encoderPosition_2 --;
  }
}
void handleB_2() {
  if (READ_B_2) {
    if (READ_A_2) encoderPosition_2 ++;
    else encoderPosition_2 --;
  } else {
    if (READ_A_2) encoderPosition_2 --;
    else encoderPosition_2 ++;
  }
}
// void handleA_2() {
//   int delta = 0;
//   if (READ_A_2) {
//     if (READ_B_2) delta = -1;
//     else delta = +1;
//   } else {
//     if (READ_B_2) delta = +1;
//     else delta = -1;
//   }
//   encoderPosition_2 += delta;
// }
// void handleB_2() {
//   int delta = 0;
//   if (READ_B_2) {
//     if (READ_A_2) delta = +1;
//     else delta = -1;
//   } else {
//     if (READ_A_2) delta = -1;
//     else delta = +1;
//   }
//   encoderPosition_2 += delta;
// }
void handleA_3() {
  int delta = 0;
  if (READ_A_3) {
    if (READ_B_3) delta = -1;
    else delta = +1;
  } else {
    if (READ_B_3) delta = +1;
    else delta = -1;
  }
  encoderRaw_3 += delta;
}
void handleB_3() {
  int delta = 0;
  if (READ_B_3) {
    if (READ_A_3) delta = +1;
    else delta = -1;
  } else {
    if (READ_A_3) delta = -1;
    else delta = +1;
  }
  encoderRaw_3 += delta;
}

////////////////////////////////////////CALCULATE ANGLE////////////////////////////////////////
// void Degree_1(float deg, float deg_old) 
// {
//   if (deg_old >= deg) 
//   {
//     digitalWrite(DIR_PIN_1, 0);
//     degree1 = abs(deg - deg_old);
//     lastDir_1_encoder = -1;
//   } 
//   else
//   {
//     digitalWrite(DIR_PIN_1, 1);
//     degree1 = abs(deg - deg_old);
//     lastDir_1_encoder = 1;
//   }
//   nPulse_1 = degree1 * 8 * pulperrev / 360;
//   if (nPulse_1 > 0) 
//   {
//     motorRunning_1 = true;
//     // startTime_1 = millis();
//   }
// }
void Degree_1(float deg, float deg_old_input) 
{
  float deg_diff = abs(deg - deg_old_input);
  if (deg_diff < 0.1) return;  // Bỏ qua nếu quá nhỏ

  if (deg_old_input >= deg) 
  {
    digitalWrite(DIR_PIN_1, 0);
    lastDir_1_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_1, 1);
    lastDir_1_encoder = 1;
  }

  degree1 = deg_diff;
  nPulse_1 = degree1 * 8 * pulperrev / 360;

  if (nPulse_1 > 0) 
  {
    motorRunning_1 = true;
    // startTime_1 = millis();
    deg1_old = deg;  // ✅ Cập nhật ở đây, chứ không phải bên ngoài
  }

  Serial.println("OK");
}
void Degree_2(float deg, float deg_old_input) 
{
  float deg_diff = abs(deg - deg_old_input);
  if (deg_diff < 0.1) return;  // Bỏ qua nếu quá nhỏ

  if (deg_old_input >= deg) 
  {
    digitalWrite(DIR_PIN_2, 0);
    lastDir_2_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_2, 1);
    lastDir_2_encoder = 1;
  }

  degree2 = deg_diff;
  nPulse_2 = degree2 * 8 * pulperrev / 360;

  if (nPulse_2 > 0) 
  {
    motorRunning_2 = true;
    deg2_old = deg;  // ✅ Cập nhật tại đây
  }
}
void Degree_3(float deg, float deg_old_input) 
{
  float deg_diff = abs(deg - deg_old_input);
  if (deg_diff < 0.1) return;  // Bỏ qua nếu quá nhỏ

  if (deg_old_input >= deg) 
  {
    digitalWrite(DIR_PIN_3, 0);
    lastDir_3_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_3, 1);
    lastDir_3_encoder = 1;
  }

  degree3 = deg_diff;
  nPulse_3 = degree3 * 8 * pulperrev / 360;

  if (nPulse_3 > 0) 
  {
    motorRunning_3 = true;
    deg3_old = deg;  // ✅ Cập nhật tại đây
  }
}

// void Degree_2(float deg, float deg_old) 
// {
//   if (deg_old >= deg) 
//   {
//     digitalWrite(DIR_PIN_2, 0);
//     degree2 = abs(deg - deg_old);
//     lastDir_2_encoder = -1;
//   } 
//   else
//   {
//     digitalWrite(DIR_PIN_2, 1);
//     degree2 = abs(deg - deg_old);
//     lastDir_2_encoder = 1;
//   }
//   nPulse_2 = degree2 * 8 * pulperrev / 360;
//   if (nPulse_2 > 0) 
//   {
//     motorRunning_2 = true;
//     // startTime_2 = millis();
//   }
// }

// void Degree_3(float deg, float deg_old) 
// {
//   if (deg_old >= deg) 
//   {
//     digitalWrite(DIR_PIN_3, 0);
//     degree3 = abs(deg - deg_old);
//     lastDir_3_encoder = -1;
//   } 
//   else
//   {
//     digitalWrite(DIR_PIN_3, 1);
//     degree3 = abs(deg - deg_old);
//     lastDir_3_encoder = 1;
//   }
//   nPulse_3 = degree3 * 8 * pulperrev / 360;
//   if (nPulse_3 > 0) 
//   {
//     motorRunning_3 = true;
//     // startTime_3 = millis();
//   }
// }

///////////////////////////////////////////RUN MOTOR//////////////////////////////////////////////
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

////////////////////////////////////////////SET HOME//////////////////////////////////////////////////
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
  // while (!yHomed)
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
    // // Set home cho motor 3
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
  Degree_1(8.5, 0); //8.5
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
  delay(500);
  // reset gốc tọa độ mới
  SetPosition_1();
  SetPosition_2();
  SetPosition_3();
  Serial.println("Da thoat khoi SetHome.");
}
void SetPosition_1()
{
  detachInterrupt(digitalPinToInterrupt(pinA_1)); 
  detachInterrupt(digitalPinToInterrupt(pinB_1));
  deg1 = 0.0;
  deg1_old = 0.0;
  degree1 = 0.0;

  encoderRawLast_1 = 0;
  encoderRawNow_1 = 0;
  delta_1 = 0;
  
  encoderPosition_1 = 0;
  encoderCalibrated_1 = 0;
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
  delta_3 = 0;

  attachInterrupt(digitalPinToInterrupt(pinA_3), handleA_3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_3), handleB_3, CHANGE);
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