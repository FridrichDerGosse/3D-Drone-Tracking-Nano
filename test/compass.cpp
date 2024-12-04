#include <Arduino.h>
#include <Wire.h> //I2C Arduino Library
//I2C address of the QMC5883L
#define ADDR  0x0d

//values for the QMC5883 control register 1
//operating mode
#define Mode_StandBy    0b00000000
#define Mode_Continuous 0b00000001
//Output data rate
#define ODR_10Hz        0b00000000
#define ODR_50Hz        0b00000100
#define ODR_100Hz       0b00001000
#define ODR_200Hz       0b00001100
//Measure range
#define RNG_2G          0b00000000
#define RNG_8G          0b00010000
//Over sampling rate
#define OSR_512         0b00000000
#define OSR_256         0b01000000
#define OSR_128         0b10000000
#define OSR_64          0b11000000

//some constants for the LED ring
#define MAXBRIGHT 64
#define LEDCOUNT 12
#define SIGNALPIN 4

//function to write data into a register on QMC5883L
void writeRegister(uint8_t reg, uint8_t val){
  Wire.beginTransmission(ADDR); //start talking
  Wire.write(reg); 
  Wire.write(val);
  Wire.endTransmission();
}

//function to read results from QMC5883L
void readData(uint16_t * x, uint16_t * y, uint16_t * z) {
  Wire.beginTransmission(ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(ADDR, 6);
  *x = Wire.read(); //LSB  x
  *x |= Wire.read() << 8; //MSB  x
  *y = Wire.read(); //LSB  z
  *y |= Wire.read() << 8; //MSB z
  *z = Wire.read(); //LSB y
  *z |= Wire.read() << 8; //MSB y  
}

//function to set the control register 1 on QMC5883L
void setCtrlRegister(uint8_t overSampling, uint8_t range, uint8_t dataRate, uint8_t mode) {
  writeRegister(9,overSampling | range | dataRate | mode);
}

//function to reset QMC5883L
void softReset() {
  writeRegister(0x0a,0x80);
  writeRegister(0x0b,0x01);
}

//prepare hardware
void setup(){
  Serial.begin(9600);
  Wire.begin();
  
  Serial.println("Start");
  softReset();
  setCtrlRegister(OSR_128,RNG_2G,ODR_100Hz,Mode_Continuous);
  Serial.println("init done");
}


void loop(){
  uint16_t x,y,z; //triple axis data
  float azimut;
  uint16_t azi; //azimut only positiv as integer
  uint8_t led1,led2;
  uint8_t val1,val2;
  int8_t diff;

  readData(&x, &y, &z); //read data from sensor
  //calculate the angle between x and y
  //wechange the signe since leds in the ring ordered clockwise   
  azimut = -atan2(y,x) * 180.0/PI;
  //add 180 degree to get only positive values
  azi = azimut+180;
  //we have 12 leds every 30 degrees
  led1=(azi/30) % 12;
  //we calculate the difference between original angle
  //and led angle gives at maximum +/- 30
  diff = azi - led1 *30;
  //we want to show both leds left and right to the
  //original angle
  if (diff>0) {
    //if difference is positiv we set the following led
    led2 = led1++;
    if (led2 > 11) led2 = 0;
    val1 = diff;
    val2 = 30-diff; 
  } else {
    //if difference is negative we set the previous led
    led2 = led1--;
    if (led2<0) led2 = 11;
    val1 = -diff;
    val2 = 30+diff;
  }


  // Show values on serial line
  Serial.print("X Value: ");
  Serial.println(x);
  Serial.print("Y Value: ");
  Serial.println(y);
  Serial.print("Z Value: ");
  Serial.println(z);
  Serial.print("Richtung: ");
  Serial.print(azimut);
  Serial.println("°");
  Serial.print("LED 1 = ");
  Serial.print(led1);
  Serial.print(" bright = ");
  Serial.println(val1);
  Serial.print("LED 2 = ");
  Serial.print(led2);
  Serial.print(" bright = ");
  Serial.println(val2);
  Serial.println();
  //wait 1 second
  delay(1000);
}