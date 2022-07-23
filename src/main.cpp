#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting");
  for(int i; i < 10; i++){
    Serial.print(".");
  }

  Serial.println(".");

  
}

void loop() {
  // put your main code here, to run repeatedly:
}