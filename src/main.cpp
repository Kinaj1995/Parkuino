#include <Arduino.h>
#include <WiFiNINA.h>

// IMU
#include <Arduino_LSM6DSOX.h>
#include <Wire.h>

// SD Card
#include "Storage.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~    Allgemeine Variabeln      ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

int loopcount0, loopcount1 = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~           SD-Card            ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

String header = "Loopcount; Pitch; Roll ;Yaw";
String dataString = "";

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~        Accelerometer         ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

void calibrateIMU(int delayMillis, int calibrationMillis);
bool readIMU();
void doCalculations();
void printCalculations();

float accelX, accelY, accelZ,                                // units m/s/s i.e. accelZ if often 9.8 (gravity)
    gyroX, gyroY, gyroZ,                                     // units dps (degrees per second)
    gyroDriftX, gyroDriftY, gyroDriftZ,                      // units dps
    accRoll, accPitch, accYaw,                               // units degrees (roll and pitch noisy, yaw not possible)
    complementaryRoll, complementaryPitch, complementaryYaw; // units degrees (excellent roll, pitch, yaw minor drift)

long lastTime;
long lastInterval;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~             Core0            ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void setup()
{

  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting");
  Serial.println(BOARD_NAME);
  for (int i = 0; i < 10; i++)
  {
    Serial.print(".");
  }

  Serial.println(".");

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
}

void loop()
{

  Serial.println("Core 0 Heartbeat");

  delay(5000);
  loopcount0++;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~             Core1            ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void setup1()
{
  delay(5000);
  Serial.println("Second Core");

  // --- Setup IMU
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  calibrateIMU(250, 250);
  lastTime = micros();

  // --- Setup SD Card
  setupStorage();
  saveFile(header);
}

void loop1()
{

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  //~~        IMU Messurement       ~~//
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

  if (readIMU())
  {
    long currentTime = micros();
    lastInterval = currentTime - lastTime; // expecting this to be ~104Hz +- 4%
    lastTime = currentTime;

    doCalculations();
    printCalculations();
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  //~~        Saving in File        ~~//
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  dataString = (String)loopcount1 + ";" + (String)complementaryRoll + ";" + (String)complementaryPitch + ";" + (String)complementaryYaw;
  // saveFile(dataString);

  loopcount1++;
  // delay(100);
}

/*
  the gyro's x,y,z values drift by a steady amount. if we measure this when arduino is still
  we can correct the drift when doing real measurements later
*/
void calibrateIMU(int delayMillis, int calibrationMillis)
{

  int calibrationCount = 0;

  delay(delayMillis); // to avoid shakes after pressing reset button

  float sumX, sumY, sumZ;
  int startTime = millis();
  while (millis() < startTime + calibrationMillis)
  {
    if (readIMU())
    {
      // in an ideal world gyroX/Y/Z == 0, anything higher or lower represents drift
      sumX += gyroX;
      sumY += gyroY;
      sumZ += gyroZ;

      calibrationCount++;
    }
  }

  if (calibrationCount == 0)
  {
    Serial.println("Failed to calibrate");
  }

  gyroDriftX = sumX / calibrationCount;
  gyroDriftY = sumY / calibrationCount;
  gyroDriftZ = sumZ / calibrationCount;
}

/**
   Read accel and gyro data.
   returns true if value is 'new' and false if IMU is returning old cached data
*/
bool readIMU()
{
  if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable())
  {
    IMU.readAcceleration(accelX, accelY, accelZ);
    IMU.readGyroscope(gyroX, gyroY, gyroZ);
    return true;
  }
  return false;
}

/**
   I'm expecting, over time, the Arduino_LSM6DS3.h will add functions to do most of this,
   but as of 1.0.0 this was missing.
*/
void doCalculations()
{
  accRoll = atan2(accelY, accelZ) * 180 / PI;
  accPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;

  float lastFrequency = (float)1000000.0 / lastInterval;

  complementaryRoll = complementaryRoll + ((gyroX - gyroDriftX) / lastFrequency);
  complementaryPitch = complementaryPitch + ((gyroY - gyroDriftY) / lastFrequency);
  complementaryYaw = complementaryYaw + ((gyroZ - gyroDriftZ) / lastFrequency);

  complementaryRoll = 0.98 * complementaryRoll + 0.02 * accRoll;
  complementaryPitch = 0.98 * complementaryPitch + 0.02 * accPitch;
}

/**
   This comma separated format is best 'viewed' using 'serial plotter' or processing.org client (see ./processing/RollPitchYaw3d.pde example)
*/
void printCalculations()
{
  Serial.print(complementaryRoll);
  Serial.print(',');
  Serial.print(complementaryPitch);
  Serial.print(',');
  Serial.print(complementaryYaw);
  Serial.println("");
}