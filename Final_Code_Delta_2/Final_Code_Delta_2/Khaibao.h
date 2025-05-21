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
#define READ_A_2 digitalRead(pinA_2)
#define READ_B_2 digitalRead(pinB_2)
#define READ_A_3 digitalRead(pinA_3)
#define READ_B_3 digitalRead(pinB_3)

const int limit_1 = 34;
const int limit_2 = 36;
const int limit_3 = 38;
const int namcham = 40;

volatile long encoderPosition_1 = 0;
volatile long encoderPosition_2 = 0;
volatile long encoderPosition_3 = 0;
volatile float encoderCalibrated_1 = 0.0;

volatile long nPulse_1 = 0;
float deg1 = 0.0;
float deg1_old = 0.0;
float degree1 = 0.0;

volatile long nPulse_2 = 0;
float deg2 = 0.0;
float deg2_old = 0.0;
float degree2 = 0.0;

volatile long nPulse_3 = 0;
float deg3 = 0.0;
float deg3_old = 0.0;
float degree3 = 0.0;

volatile bool motorRunning_1 = false;
volatile bool motorRunning_2 = false;
volatile bool motorRunning_3 = false;

String inString = "";


unsigned long startTime_1 = 0,startTime_2 = 0,startTime_3 = 0;
unsigned long endTime_1 = 0,endTime_2 = 0,endTime_3 = 0;
unsigned long lastPrintTime = 0;


// Số xung = góc*8(tỉ số truyền)*200(xung/vòng)/360 -> 1(xung) = góc*8*200/360 -> 1 xung sẽ được 0.225 độ 
// 4000us(1 xung) = 0.225độ vậy  ?us = 30 độ
// Vậy thì với tf = 2s thì từ 0 độ tới 30độ hết 2s thì cần ?us cho 1 xung
unsigned long delay_run_spd_1 = 2000;
unsigned long delay_run_spd_2 = 2000;
unsigned long delay_run_spd_3 = 2000;

unsigned long delay_home_spd = 5000;
unsigned long pulperrev = 200;

volatile unsigned long lastToggleTime_1 = 0,lastToggleTime_2 = 0,lastToggleTime_3 = 0;
// volatile unsigned long lastPulseTime_1 = 0,lastPulseTime_2 = 0,lastPulseTime_3 = 0;

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

unsigned long currentMillis;
unsigned long prevMainLoop = 0;
unsigned long prevTrajectory = 0;
const unsigned long intervalMainLoop = 20;      // 50 Hz
const unsigned long intervalTrajectory = 10;    // 100 Hz

// Biến cho quy hoạch quỹ đạo
float t = 0.0;
float t0 = 0.0;
// float tf = 2.0;  // thời gian di chuyển tổng cộng (giây)
float P0[3], Pf[3], tf;
float dt = 0.01;  // bước thời gian (giây)
bool trajectory_ready = false; // Đánh dấu đã nhận xong dữ liệu quỹ đạo

// Vị trí ban đầu và cuối cùng
float x0, y0, z0;   // Điểm đầu
float xf, yf, zf;   // Điểm cuối
// float tf;           // Thời gian di chuyển (giây)
unsigned long startTime;  // Thời điểm bắt đầu di chuyển (ms)
bool moveInProgress = false;
