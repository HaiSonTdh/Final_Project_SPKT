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
    deg1_old = deg;  
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
    deg2_old = deg; 
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
    deg3_old = deg;  
  }
}