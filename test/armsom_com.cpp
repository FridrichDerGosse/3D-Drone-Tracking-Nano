/**
 * @brief armsom test
 *
 */
#include <Wire.h>
#include <Arduino.h>
// i2c/wiring proof of concept code - tie two Arduinos together via
// i2c and do some simple data xfer.
//
// slave side code
//
// Jason Winningham (kg4wsv)
// 14 jan 2008

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void requestEvent()
{
  Serial.println("request");
  static char c = '0';

  Wire.write(c++);
  if (c > 'z')
    c = '0';

  Serial.println("wrote");
}

void setup()
{
  Serial.begin(9600);
  Serial.println("setup");
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event

  Serial.println("setup done");
}

void loop()
{
  delay(100);
}