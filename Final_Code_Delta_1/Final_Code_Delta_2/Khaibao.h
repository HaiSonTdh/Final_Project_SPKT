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
const int conveyor = 42;

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

unsigned long delay_run_spd = 5000;
unsigned long delay_home_spd = 5000;
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
