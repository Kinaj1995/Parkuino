#include <Arduino.h>
#include <WiFiNINA.h>
#include <Arduino_LSM6DSOX.h>

int loopcount = 0;

float Ax, Ay, Az;
float Gx, Gy, Gz;


// Accelerometer
const float alpha = 0.5;

double fXg = 0;
double fYg = 0;
double fZg = 0;

double refXg = 0;
double refYg = 0;
double refZg = 0;

int y = 0;




void setup()
{

  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting");
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
}

void loop()
{
  loopcount++;

  delay(100);
}

void setup1()
{
  Serial.println("Second Core");
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1)
      ;
  }

  /*
    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println("Hz");
    Serial.println();

    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println("Hz");
    Serial.println();
    */
}

void loop1()
{

  
  if (IMU.accelerationAvailable()) {


    IMU.readAcceleration(Ax, Ay, Az);
    /*
    Serial.println("Accelerometer data: ");
    Serial.print(Ax);
    Serial.print('\t');
    Serial.print(Ay);
    Serial.print('\t');
    Serial.println(Az);
    Serial.println();
    */
  }
  

  if (IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(Gx, Gy, Gz);

    /* Serial.println("Gyroscope data: ");
    Serial.println(Gx);

    
    Serial.print('\t');
    Serial.print(Gy);
    Serial.print('\t');
    Serial.println(Gz);
    Serial.println();
    */
  }


  // ADXL sensor
  double pitch, roll;
 

  // Calibration ADXL345
  if (y == 0)
  {
    refXg = Ax; refYg = Ay; refZg = Az;
    y = 1;
  }

  Ax = Ax - refXg;
  Ay = Ay - refYg;
  Az = Az - refZg + 1;

  fXg = Ax * alpha + (fXg * (1.0 - alpha));
  fYg = Ay * alpha + (fYg * (1.0 - alpha));
  fZg = Az * alpha + (fZg * (1.0 - alpha));

  // Roll & Pitch Equations
  roll  = -(atan2(-fYg, fZg) * 180.0) / PI; 
  pitch = (atan2(fXg, sqrt(fYg * fYg + fZg * fZg)) * 180.0) / PI;


  Serial.printf("Lean: %f, Wheele: %f \n", roll, pitch);

 


  delay(500);
}
