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

void setup() 
{ 
  Wire.begin(2);                // join i2c bus with address #2 
  Wire.onRequest(requestEvent); // register event 
  Serial.begin(9600);           // start serial for output 
} 

void loop() 
{ 
  delay(100); 
} 

// function that executes whenever data is received from master 
// this function is registered as an event, see setup() 
void requestEvent() 
{
  static char c = '0';

  Wire.write(c++);
  if (c > 'z')
    c = '0';
}
