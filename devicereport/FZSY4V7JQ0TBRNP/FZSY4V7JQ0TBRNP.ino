//This sketch is used to calibrate the raw magnetometer and accelerometer data provided by the Adafruit LSM303DHLC sensor. With the help of these measured values one obtains
//a tilt compensated digital compass.
//A description of the calculation steps is given in the ST Application Note AN4508
//https://www.st.com/content/ccc/resource/technical/document/application_note/a0/f0/a0/62/3b/69/47/66/DM00119044.pdf/files/DM00119044.pdf/jcr:content/translations/en.DM00119044.pdf
//For the procedure for calibrating the sensor, see https://diydrones.com/profiles/blogs/advanced-hard-and-soft-iron-magnetometer-calibration-for-dummies
//or https://www.instructables.com/id/Easy-hard-and-soft-iron-magnetometer-calibration/
//Use my sketch file LSM303DHLC_Acc-and_Mag_Raw_Measurements_201218 for calibrating.
//Thanks to Yury Matselenak and Chris H for their contributions!


#include "Wire.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

#include <time.h>
//#include <LiquidCrystal.h>

Adafruit_LSM303 lsm;
//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float Mx, My, Mz; // Raw Magnetometer measurements
float Mxc, Myc, Mzc; // Calibrated Magnetometer measurements
float Mxcn, Mycn, Mzcn; // Normalized Magnetometer measurements


float Ax, Ay, Az; // Raw Accelerometer measurments
float Axc, Ayc, Azc;  // Calibrated Accelerometer mesurements
float Axcn, Aycn, Azcn; // Normalized Accelerometer measurements

//https://content.instructables.com/ORIG/FGL/HY0Y/JPX5CDGB/FGLHY0YJPX5CDGB.jpg?auto=webp&frame=1&fit=bounds&md=23ee8193b659d7d3d28443d61348c5c8
float norm_a, norm_m;

double pitch, roll;
double pitch_print, roll_print;
double tiltcnf;

double Xh, Yh;  // Tilt compensated values for heading calculation
float headingct; // Calibrated and tilt compensated heading


//calibrated_values[3] is the global array where the calibrated data will be placed
//calibrated_values[3]: [0]=Xc, [1]=Yc, [2]=Zc
float calibrated_mag_values[3];
float calibrated_acc_values[3];

float alpha = 0.25; // Low-Pass Filtercoefficient

// Filtered Magnetometer measurements
float Mxcnf = 0;
float Mycnf = 0;
float Mzcnf = 0;

// Filtered Accelerometer measurements
float Axcnf = 0;
float Aycnf = 0;
float Azcnf = 0;


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  lsm.begin();
//  lcd.begin(16, 2);
}

void loop()
{
  clock_t start = clock();
  lsm.read();

  float values_from_magnetometer[3];
  float values_from_accelerometer[3];

  getRawValuesMag();

  values_from_magnetometer[0] = Mx;
  values_from_magnetometer[1] = My;
  values_from_magnetometer[2] = Mz;

  transformation_mag(values_from_magnetometer);

  Mxc = calibrated_mag_values[0];
  Myc = calibrated_mag_values[1];
  Mzc = calibrated_mag_values[2];

  getRawValuesAcc();

  values_from_accelerometer[0] = Ax;
  values_from_accelerometer[1] = Ay;
  values_from_accelerometer[2] = Az;

  transformation_acc(values_from_accelerometer);

  Axc = calibrated_acc_values[0];
  Ayc = calibrated_acc_values[1];
  Azc = calibrated_acc_values[2];

  //  Normalizing
  norm_m = sqrt(sq(Mxc) + sq(Myc) + sq(Mzc)); //original code did not appear to normalize, and this seems to help
  Mxcn = Mxc / norm_m;
  Mycn = Myc / norm_m;
  Mzcn = Mzc / norm_m;

  norm_a = sqrt(sq(Ax) + sq(Ay) + sq(Az)); //original code did not appear to normalize, and this seems to help
  Axcn = Axc / norm_a;
  Aycn = Ayc / norm_a;
  Azcn = Azc / norm_a;


  // Low-Pass filter magnetometer
  Mxcnf = Mxcn * alpha + (Mxcnf * (1.0 - alpha));
  Mycnf = Mycn * alpha + (Mycnf * (1.0 - alpha));
  Mzcnf = Mzcn * alpha + (Mzcnf * (1.0 - alpha));

  // Low-Pass filter accelerometer
  Axcnf = Axcn * alpha + (Axcnf * (1.0 - alpha));
  Aycnf = Aycn * alpha + (Aycnf * (1.0 - alpha));
  Azcnf = Azcn * alpha + (Azcnf * (1.0 - alpha));


  // Calculating pitch and roll angles following Application Note
  pitch = (double) asin((double) - Axcnf);
  pitch_print = pitch * 180 / PI;
  roll = (double) asin((double) Aycnf / cos((double) pitch));
  roll_print = roll * 180 / PI;


  //  Calculating heading with raw measurements not tilt compensated
  float heading = (double)atan2((double)My, (double)Mx) * 180 / PI;
  if (heading < 0) {
    heading = 360 + heading;
  }

  //  Calculating heading with calibrated measurements not tilt compensated
  float headingc = (double)atan2((double)Myc, (double)Mxc) * 180 / PI;
  if (headingc < 0) {
    headingc = 360 + headingc;
  }

  //  Calculating tilt compensated heading
  Xh = Mxcnf * cos((double)pitch) + Mzcnf * sin((double)pitch);
  Yh = Mxcnf * sin((double)roll) * sin((double)pitch) + Mycnf * cos((double)roll) - Mzcnf * sin((double)roll) * cos((double)pitch);
  headingct = (atan2(Yh, Xh)) * 180 / PI;
  if (headingct < 0) {
    headingct = 360 + headingct;
  }


  //Calculating Tilt angle in degrees
  tiltcnf = (double)atan2((double)fabs(Azcnf), (double)Axcnf) * 180 / PI;

  Serial.flush();

  // Display calibrated accelerometer measurements in milliG
  Serial.print(Axc); Serial.print(" ");
  Serial.print(Ayc); Serial.print(" ");
  Serial.print(Azc); Serial.print(" ");

  // Display Tilt angle in degrees
  // Serial.print(tiltcnf); Serial.print(" ");

  // Display Heading in degrees North = 0°--> 360° turning clockwise
  Serial.print (headingct); Serial.print(" ");
  
  // Display Pitch and Roll angles in degrees
  Serial.print (pitch_print); Serial.print(" ");
  Serial.print (roll_print); Serial.print(" ");

  printf(" %.3f %.3f %.3f %.3f %.3f %.3f ", Axc, Ayc, Azc, headingct, pitch_print, roll_print);

  clock_t end = clock();
  double elapsed = double(end - start)/CLOCKS_PER_SEC;
  printf("%.3f \n", elapsed);
}

// Read the raw measurements
void getRawValuesMag()
{
  Mx = (float)lsm.magData.x;
  My = (float)lsm.magData.y;
  Mz = (float)lsm.magData.z;
}

void getRawValuesAcc()
{
  Ax = (int)lsm.accelData.x;
  Ay = (int)lsm.accelData.y;
  Az = (int)lsm.accelData.z;
}


//transformation(float uncalibrated_values[3]) is the function of the magnetometer data correction
//uncalibrated_values[3] is the array of the non calibrated magnetometer data
//uncalibrated_values[3]: [0]=Xnc, [1]=Ync, [2]=Znc

void transformation_mag(float uncalibrated_values[3])
{
  //calibration_matrix[3][3] is the transformation matrix
  //replace M11, M12,..,M33 with your transformation matrix data
  double calibration_matrix[3][3] =
  {
    {1.025, 0.026, 0.004},
    {0.003, 1.024, 0.002},
    {0.018, -0.002, 1.251}
  };
  //bias[3] is the bias
  //replace Bx, By, Bz with your bias data
  double bias[3] =
  {
    -6.278,
    -101.76,
    -179.707
  };
  //calculation
  for (int i = 0; i < 3; ++i) uncalibrated_values[i] = uncalibrated_values[i] - bias[i];
  float result[3] = {0, 0, 0};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      result[i] += calibration_matrix[i][j] * uncalibrated_values[j];
  for (int i = 0; i < 3; ++i) calibrated_mag_values[i] = result[i];
}


//transformation(float uncalibrated_values[3]) is the function of the accelerometer data correction
//uncalibrated_values[3] is the array of the non calibrated magnetometer data
//uncalibrated_values[3]: [0]=Xnc, [1]=Ync, [2]=Znc

void transformation_acc(float uncalibrated_values[3])
{
  //calibration_matrix[3][3] is the transformation matrix
  //replace ACC11(M11), ACC12(M12),..,ACC33(M33) with your transformation matrix data
  double calibration_matrix[3][3] =
  {
    {0.98, 0.002, 0.001},
    { -0.005, 1.003, 0.007},
    {0.011, -0.005, 1.019}
  };
  //zero-g[3] is the zero-g offset
  //replace ACC10(Bx), ACC20(By), ACC30(Bz) with your zero-g offset data
  double zero_g[3] =
  {
    -30.988,
    10.754,
    -8.697
  };
  //calculation
  for (int i = 0; i < 3; ++i) uncalibrated_values[i] = uncalibrated_values[i] - zero_g[i];
  float result[3] = {0, 0, 0};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      result[i] += calibration_matrix[i][j] * uncalibrated_values[j];
  for (int i = 0; i < 3; ++i) calibrated_acc_values[i] = result[i];
}
