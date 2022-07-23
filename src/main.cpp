#include <Arduino.h>

int loopcount = 0;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
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
}

void loop()
{
  loopcount++;

  Serial.println(loopcount);
  delay(100);
}