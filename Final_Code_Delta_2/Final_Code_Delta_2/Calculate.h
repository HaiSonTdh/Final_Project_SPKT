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
  Serial.print("Motor 1 - nPulse: "); Serial.println(nPulse_1);
  if (nPulse_1 > 0) 
  {
    motorRunning_1 = true;
    // startTime_1 = millis();
  }
  // Serial.println("OK");
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
  Serial.print("Motor 2 - nPulse: "); Serial.println(nPulse_2);
  if (nPulse_2 > 0) 
  {
    motorRunning_2 = true;
    // startTime_2 = millis();
  }
  // Serial.println("OK");
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
  Serial.print("Motor 3 - nPulse: "); Serial.println(nPulse_3);
  if (nPulse_3 > 0) 
  {
    motorRunning_3 = true;
    // startTime_3 = millis();
  }
  // Serial.println("OK");
}

bool inverse_kinematic(float X_ee, float Y_ee, float Z_ee, float *J)
{
  const float R_Base = 60.0;
  const float R_platform = 42.62;
  const float r = R_Base - R_platform;
  const float re = 150.0;
  const float rf = 350.0;
  const float threshold = 0.001;
  const float alpha_deg[3] = {0, 120, 240};

  for (int i = 0; i < 3; i++) {
    float alpha = radians(alpha_deg[i]); // Chuyển alpha từ độ sang radian
    float cos_alpha = cos(alpha);
    float sin_alpha = sin(alpha);

    float A = -2.0 * re * (-r + X_ee * cos_alpha + Y_ee * sin_alpha);
    float B = -2.0 * re * Z_ee;
    float C = (X_ee * X_ee + Y_ee * Y_ee + Z_ee * Z_ee + r * r + re * re - rf * rf
              - 2 * r * (X_ee * cos_alpha + Y_ee * sin_alpha));

    float denominator = sqrt(A * A + B * B);
    if (denominator < 1e-6 || fabs(C / denominator) > 1.0) {
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
// void trajectory_planning_2_point(float t, float* P0, float* Pf, float tf, float* x, float* y, float* z) {
//   float delta_x = Pf[0] - P0[0];
//   float delta_y = Pf[1] - P0[1];
//   float delta_z = Pf[2] - P0[2];

//   float a0x = P0[0];
//   float a1x = 0;
//   float a2x = 0;
//   float a3x = 10 * delta_x / pow(tf, 3);
//   float a4x = -15 * delta_x / pow(tf, 4);
//   float a5x = 6 * delta_x / pow(tf, 5);

//   float a0y = P0[1];
//   float a1y = 0;
//   float a2y = 0;
//   float a3y = 10 * delta_y / pow(tf, 3);
//   float a4y = -15 * delta_y / pow(tf, 4);
//   float a5y = 6 * delta_y / pow(tf, 5);

//   float a0z = P0[2];
//   float a1z = 0;
//   float a2z = 0;
//   float a3z = 10 * delta_z / pow(tf, 3);
//   float a4z = -15 * delta_z / pow(tf, 4);
//   float a5z = 6 * delta_z / pow(tf, 5);

//   if (t <= 0) {
//     *x = P0[0];
//     *y = P0[1];
//     *z = P0[2];
//   } else if (t <= tf) {
//     *x = a0x + a1x*t + a2x*t*t + a3x*t*t*t + a4x*t*t*t*t + a5x*t*t*t*t*t;
//     *y = a0y + a1y*t + a2y*t*t + a3y*t*t*t + a4y*t*t*t*t + a5y*t*t*t*t*t;
//     *z = a0z + a1z*t + a2z*t*t + a3z*t*t*t + a4z*t*t*t*t + a5z*t*t*t*t*t;
//   } else {
//     *x = Pf[0];
//     *y = Pf[1];
//     *z = Pf[2];
//   }
// }

// Thêm define số đoạn chia quỹ đạo
#define SEGMENT_DIVISIONS 20  // Chia quỹ đạo thành 20 đoạn nhỏ

void trajectory_planning_2_point(float t, float* P0, float* Pf, float tf, float* x, float* y, float* z) {
  static float last_x = P0[0];
  static float last_y = P0[1];
  static float last_z = P0[2];
  static float segment_time = tf / SEGMENT_DIVISIONS;
  static int current_segment = 0;
  
  // Reset khi bắt đầu quỹ đạo mới
  if (t <= 0) {
    last_x = P0[0]; //lưu vị trí cũ
    last_y = P0[1];
    last_z = P0[2];
    current_segment = 0;
  }

  if (t >= tf) {
    *x = Pf[0];
    *y = Pf[1];
    *z = Pf[2];
    StopMotor_1();
    StopMotor_2();
    StopMotor_3();
    return;
  }

  // Tính toán VỊ TRÍ lý tưởng theo phương trình gốc
  float delta_x = Pf[0] - P0[0];
  float delta_y = Pf[1] - P0[1];
  float delta_z = Pf[2] - P0[2];

  float target_x = P0[0] + delta_x * (10*pow(t/tf,3) - 15*pow(t/tf,4) + 6*pow(t/tf,5));
  float target_y = P0[1] + delta_y * (10*pow(t/tf,3) - 15*pow(t/tf,4) + 6*pow(t/tf,5));
  float target_z = P0[2] + delta_z * (10*pow(t/tf,3) - 15*pow(t/tf,4) + 6*pow(t/tf,5));

  // Chia thành các đoạn nhỏ
  float segment_progress = fmod(t, segment_time) / segment_time;
  //fmod trả về phần dư của phép chia số thực t/segment_time
  // Nếu sang đoạn mới
  if ((int)(t / segment_time) > current_segment) {
    current_segment = (int)(t / segment_time);
    last_x = *x;
    last_y = *y;
    last_z = *z;
  }

  // Nội suy tuyến tính giữa các đoạn
  *x = last_x + (target_x - last_x) * segment_progress;
  *y = last_y + (target_y - last_y) * segment_progress;
  *z = last_z + (target_z - last_z) * segment_progress;

  // // Debug thông tin
  // static unsigned long last_print = 0;
  // if (millis() - last_print > 100) {
  //   last_print = millis();
  //   Serial.print("Segment: "); Serial.print(current_segment);
  //   Serial.print(" Target: "); Serial.print(target_x); Serial.print(","); Serial.print(target_y); Serial.print(","); Serial.print(target_z);
  //   Serial.print(" Actual: "); Serial.print(*x); Serial.print(","); Serial.print(*y); Serial.print(","); Serial.println(*z);
  // }
}