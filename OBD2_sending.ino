/*************************************************************************
* Sample sketch based on OBD-II library for Arduino
* Distributed under GPL v2.0
* Visit http://freematics.com for more information
* (C)2012-2014 Stanley Huang <stanleyhuangyc@gmail.com>
*************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <OBD.h>

COBDI2C obd;

void setup()
{
  Serial.begin(115200);
  // we'll use the debug LED as output
  pinMode(13, OUTPUT);  
  // start communication with OBD-II UART adapter
  obd.begin();
  // initiate OBD-II connection until success
  while (!obd.init()){
    Serial.println("obd initilizing...");
  }
  Serial.println("function setup is end.");
}

void loop()
{
  int value;
  if (obd.readPID(PID_RPM, value)) {
    // RPM is successfully read and its value stored in variable 'value'
    // light on LED when RPM exceeds 3000
    Serial.println("rpm: " + value);
    digitalWrite(13, value > 2000 ? HIGH : LOW);
  }
}
