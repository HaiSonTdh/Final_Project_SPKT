void runToAngle(int motorID, float angle) 
{
  switch (motorID) {
    case 1: Degree_1(angle, deg1_old);  break;
    case 2: Degree_2(angle, deg2_old);  break;
    case 3: Degree_3(angle, deg3_old);  break;
  }
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
// indexOf: tính tổng các kí tự trc khi thấy kí tự |, vị trí bắt đầu là 0
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
  // float theta_start[3], theta_end[3];
  // if (!inverse_kinematic(P0[0], P0[1], P0[2], theta_start)) {
  //   Serial.println("Lỗi IK tại P0");
  //   return false;
  // }
  // if (!inverse_kinematic(Pf[0], Pf[1], Pf[2], theta_end)) {
  //   Serial.println("Lỗi IK tại Pf");
  //   return false;
  // }

  // float d1 = abs(theta_end[0] - theta_start[0]);
  // float d2 = abs(theta_end[1] - theta_start[1]);
  // float d3 = abs(theta_end[2] - theta_start[2]);

  // float pulses_1 = d1 * pulperrev * 8 / 360.0;
  // float pulses_2 = d2 * pulperrev * 8 / 360.0;
  // float pulses_3 = d3 * pulperrev * 8 / 360.0;

  // float t1 = pulses_1 * delay_run_spd / 1000000.0;
  // float t2 = pulses_2 * delay_run_spd / 1000000.0;
  // float t3 = pulses_3 * delay_run_spd / 1000000.0;

  // tf = max(t1, max(t2, t3));  // Gán lại tf
  tf = 5;
  // In kiểm tra
  // Serial.print("Tự tính tf = "); Serial.println(tf, 4);
  // Serial.print("P0 = "); Serial.print(P0[0]); Serial.print(", "); Serial.print(P0[1]); Serial.print(", "); Serial.println(P0[2]);
  // Serial.print("Pf = "); Serial.print(Pf[0]); Serial.print(", "); Serial.print(Pf[1]); Serial.print(", "); Serial.println(Pf[2]);

  return true; 
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