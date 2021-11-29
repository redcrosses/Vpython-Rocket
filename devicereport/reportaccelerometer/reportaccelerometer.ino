/*
The sensor outputs provided by the library are the raw 16-bit values
obtained by concatenating the 8-bit high and low accelerometer and
magnetometer data registers. They can be converted to units of g and
gauss using the conversion factors specified in the datasheet for your
particular device and full scale setting (gain).

Example: An LSM303D gives a magnetometer X axis reading of 1982 with
its default full scale setting of +/- 4 gauss. The M_GN specification
in the LSM303D datasheet (page 10) states a conversion factor of 0.160
mgauss/LSB (least significant bit) at this FS setting, so the raw
reading of -1982 corresponds to 1982 * 0.160 = 317.1 mgauss =
0.3171 gauss.

In the LSM303DLHC, LSM303DLM, and LSM303DLH, the acceleration data
registers actually contain a left-aligned 12-bit number, so the lowest
4 bits are always 0, and the values should be shifted right by 4 bits
(divided by 16) to be consistent with the conversion factors specified
in the datasheets.

Example: An LSM303DLH gives an accelerometer Z axis reading of -16144
with its default full scale setting of +/- 2 g. Dropping the lowest 4
bits gives a 12-bit raw value of -1009. The LA_So specification in the
LSM303DLH datasheet (page 11) states a conversion factor of 1 mg/digit
at this FS setting, so the value of -1009 corresponds to -1009 * 1 =
1009 mg = 1.009 g.
*/

#include <Wire.h>
#include <LSM303.h>

#include <stdio.h>
#include <time.h>
//#include <SimpleKalmanFilter.h>

#include <Wire.h>
#include <Adafruit_BMP280.h>

#define SEALEVELPRESSURE_HPA (997) //make sure to calibrate this value before launch, according to local forecast!

Adafruit_BMP280 bmp;

LSM303 compass;

char report[80];

float pitch = 0;
float roll = 0;
float yaw = 0;

float alt = 0;
float temp = 0;
float pressure = 0;

int mag_x = 0;
int mag_y = 0;

void setup()
{
  Serial.begin(9600);
//  if (!compass.begin(0x19)) {
//    Serial.println("Could not find a valid LSM303DLHC accelerometer, check wiring!");
//    while (1);
//  }
  Wire.begin();
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  
  compass.init();
  compass.enableDefault();

  compass.m_min = (LSM303::vector<int16_t>){-32767, -32767, -32767};
  compass.m_max = (LSM303::vector<int16_t>){+32767, +32767, +32767};

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  
}

void loop()
{
//  SimpleKalmanFilter simpleKalmanFilter(0.05, 0.05, 0.01);
  alt = bmp.readAltitude(SEALEVELPRESSURE_HPA); //in m
  temp = bmp.readTemperature(); //*C
  pressure = bmp.readPressure(); // Pa

//  clock_t start = clock();
  compass.read();

//  snprintf(report, sizeof(report), "%6d %6d %6d %6d %6d %6d %6d %6d   ",
//    compass.a.x, compass.a.y, alt,
//    pitch, roll, yaw, 
//    temp, pressure);
    Serial.println(compass.a.x, compass.a.y); Serial.print(alt, pitch, roll, yaw, temp, pressure);
//    compass.a.x = simpleKalmanFilter.updateEstimate(compass.a.x);
//    compass.a.y = simpleKalmanFilter.updateEstimate(compass.a.y);
//    compass.a.z = simpleKalmanFilter.updateEstimate(compass.a.z);
//    compass.m.x = simpleKalmanFilter.updateEstimate(compass.m.x);
//    compass.m.y = simpleKalmanFilter.updateEstimate(compass.m.y);
//    compass.m.z = simpleKalmanFilter.updateEstimate(compass.m.z);

    pitch = 180 * atan2(compass.a.x, sqrt(compass.a.y*compass.a.y + compass.a.z*compass.a.z))/PI;
    roll = 180 * atan2(compass.a.y, sqrt(compass.a.x*compass.a.x + compass.a.z*compass.a.z))/PI;
//    mag_x = compass.m.x*cos(pitch) + compass.m.y*sin(roll)*sin(pitch) + compass.m.z*cos(roll)*sin(pitch);
//    mag_y = compass.m.y * cos(roll) - compass.m.z * sin(roll);
    yaw = compass.heading(); //180 * atan2(-mag_y,mag_x)/M_PI;

//    pitch = simpleKalmanFilter.updateEstimate(pitch);
//    roll = simpleKalmanFilter.updateEstimate(roll);
//    yaw = simpleKalmanFilter.updateEstimate(yaw);

    
    
  Serial.println(report);

//  delay(100);
//  clock_t end = clock();
//  double elapsed = double(end - start)/CLOCKS_PER_SEC;
//  printf("Time measured: %.3f seconds.\n", elapsed);
}
