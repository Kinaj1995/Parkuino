#include <Arduino.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DSOX.h>

// Test SD Card
#include <SPI.h>
#include <SD.h>

File root;

void printDirectory(File dir, int numTabs);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~    Allgemeine Variabeln      ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

float Ax, Ay, Az;
float Gx, Gy, Gz;

int loopcount0, loopcount1 = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~        Accelerometer         ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

const float alpha = 0.5;

double fXg = 0;
double fYg = 0;
double fZg = 0;

double refXg = 0;
double refYg = 0;
double refZg = 0;

double pitch, roll;

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

  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);

  // Test SD

  Serial.print("Initializing SD card...");

  if (!SD.begin(SS))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");
}

void loop()
{

  /*
  Serial.printf("Lean: %f, Wheele: %f \n", roll, pitch);

  delay(500);
  loopcount0++;
  */
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//~~             Core1            ~~//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void setup1()
{ 

  
  Serial.println("Second Core");
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }
  
}

void loop1()
{

  if (IMU.accelerationAvailable())
  {
    IMU.readAcceleration(Ax, Ay, Az);

    // Calibration, noch anpassen!
    if (loopcount1 < 1)
    {
      refXg = Ax;
      refYg = Ay;
      refZg = Az;
    }

    // Conversion to angles
    Ax = Ax - refXg;
    Ay = Ay - refYg;
    Az = Az - refZg + 1;

    fXg = Ax * alpha + (fXg * (1.0 - alpha));
    fYg = Ay * alpha + (fYg * (1.0 - alpha));
    fZg = Az * alpha + (fZg * (1.0 - alpha));

    // Roll & Pitch Equations
    roll = -(atan2(-fYg, fZg) * 180.0) / PI;
    pitch = (atan2(fXg, sqrt(fYg * fYg + fZg * fZg)) * 180.0) / PI;
  }

  if (IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(Gx, Gy, Gz);
  }

  delay(100);
  loopcount1++;
}



void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm * tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}
