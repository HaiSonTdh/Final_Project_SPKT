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

unsigned long delay_run_spd = 7000;
unsigned long delay_home_spd = 7000;
unsigned long pulperrev = 200;

volatile int lastDir_1_encoder = 1;  // 1 = thuận, -1 = nghịch
volatile int lastDir_2_encoder = 1;  // 1 = thuận, -1 = nghịch
volatile int lastDir_3_encoder = 1; // 1 = thuận, -1 = nghịch
long encoderRawLast_1 = 0;
long encoderRawNow_1 = 0;
long delta_1 = 0;
volatile long encoderRaw_2 = 0;
long lastEncoderRaw_2 = 0;    
volatile long encoderRaw_3 = 0;      // Encoder chưa hiệu chỉnh (raw, đọc trong ISR)
long lastEncoderRaw_3 = 0;           // Dùng để tính delta ngoài ISR

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
  // while (Serial.available()) 
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
  long delta_3 = rawNow_3 - lastEncoderRaw_3;
  lastEncoderRaw_3 = rawNow_3;

  if (lastDir_3_encoder == -1)  // quay ngược
      encoderPosition_3 += delta_3 * 2;
  else
      encoderPosition_3 += delta_3;

  if (millis() - lastPrintTime >= 2000) 
  { 
      Serial.print("Encoder 1: "); Serial.print(encoderPosition_1);  Serial.print("  ");
      Serial.print("Encoder 2: "); Serial.print(encoderPosition_2);  Serial.print("  ");
      Serial.print("Encoder 3: "); Serial.println(encoderPosition_3); 
      // (long)encoderCalibrated_1 
      lastPrintTime = millis();
  }
  RunMotor_1();
  RunMotor_2();
  RunMotor_3();
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
    if (READ_B_2) encoderPosition_2 -= 2;
    else encoderPosition_2 += 1;
  } else {
    if (READ_B_2) encoderPosition_2 += 1;
    else encoderPosition_2 -= 2;
  }
}
void handleB_2() {
  if (READ_B_2) {
    if (READ_A_2) encoderPosition_2 += 1;
    else encoderPosition_2 -= 2;
  } else {
    if (READ_A_2) encoderPosition_2 -= 2;
    else encoderPosition_2 += 1;
  }
}

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
void Degree_1(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_1, 0);
    degree1 = abs(deg - deg_old);
    lastDir_1_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_1, 1);
    degree1 = abs(deg - deg_old);
    lastDir_1_encoder = 1;
  }
  nPulse_1 = degree1 * 8 * pulperrev / 360;
  if (nPulse_1 > 0) 
  {
    motorRunning_1 = true;
    startTime_1 = millis();
  }
  Serial.println("OK");
}

void Degree_2(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_2, 0);
    degree2 = abs(deg - deg_old);
    lastDir_2_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_2, 1);
    degree2 = abs(deg - deg_old);
    lastDir_2_encoder = 1;
  }
  nPulse_2 = degree2 * 8 * pulperrev / 360;
  if (nPulse_2 > 0) 
  {
    motorRunning_2 = true;
    startTime_2 = millis();
  }
  Serial.println("OK");
}

void Degree_3(float deg, float deg_old) 
{
  if (deg_old >= deg) 
  {
    digitalWrite(DIR_PIN_3, 0);
    degree3 = abs(deg - deg_old);
    lastDir_3_encoder = -1;
  } 
  else
  {
    digitalWrite(DIR_PIN_3, 1);
    degree3 = abs(deg - deg_old);
    lastDir_3_encoder = 1;
  }
  nPulse_3 = degree3 * 8 * pulperrev / 360;
  if (nPulse_3 > 0) 
  {
    motorRunning_3 = true;
    startTime_3 = millis();
  }
  Serial.println("OK");
}

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
  endTime_1 = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime_1 - startTime_1;
  Serial.print("Run time 1: ");
  Serial.print(runDuration);
  Serial.println(" ms");
}  
void StopMotor_2() 
{
  motorRunning_2 = false;
  digitalWrite(PUL_PIN_2, LOW);
  endTime_2 = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime_2 - startTime_2;
  Serial.print("Run time 2: ");
  Serial.print(runDuration);
  Serial.println(" ms");
}  
void StopMotor_3() 
{
  motorRunning_3 = false;
  digitalWrite(PUL_PIN_3, LOW);
  endTime_3 = millis(); // Ghi lại thời gian kết thúc
  unsigned long runDuration = endTime_3 - startTime_3;
  Serial.print("Run time 3: ");
  Serial.print(runDuration);
  Serial.println(" ms");
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
  // while (!xHomed)
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

  attachInterrupt(digitalPinToInterrupt(pinA_3), handleA_3, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB_3), handleB_3, CHANGE);
}

