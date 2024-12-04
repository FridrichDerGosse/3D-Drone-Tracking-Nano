#include <Arduino.h>
#include <SoftwareSerial.h>
#include "laser.hpp"


/**
 * source: https://dfimg.dfrobot.com/nobody/wiki/068db268ba37a41067c1b17607932139.pdf
 * - Protocol
 * 9600 baud rate, 8 data bits, 1 start bit, 1 stop bit, no parity
 * 
 * -- Command Messages
 * <ADDR> <GRP> <CMD> <CS>
 * <ADDR> <GRP> <CMD> <user option> <CS>
 * 
 * --- commands (all with broadcast=FA)
 * 0x04: {
 *  - 0x01: set address, needs: <ADDR>
 *  - 0x02: shutdown, needs: <ADDR>
 *  - 0x05: set data return interval (for continuous measurement), needs: <mealinterval>
 *  - 0x06: revise distance, needs: <symbol, -/+ = 0x2d/0x2b> <revised value, one byte>
 *  - 0x08: set distance start and end point, needs: <position=1(top),0(tail)>
 *  - 0x09: set range, needs: <range=05,10,30,50,80m>
 *      +: FA 04 89 79
 *      -: FA 84 89 01 F8
 *  - 0x0A: set frequency, needs: <frequency=05,10,20>
 *  - 0x0C: set resolution, needs: <resolution=1(1mm),2(0.1mm)>
 *  - 0x0D: set measurement starts on power on, needs: <start=0(disable),1(enable)>
 * }
 * 0x06: {
 *  - 0x01: read parameter  <CS=FF>
 *  - 0x02: single measurement (1mm), needs: <ADDR>, <CS>
 *      +: ADDR 06 82”3X 3X 3X 2E 3X 3X 3X”CS
 *      -: ADDR 06 82”’E’ ’R’ ’ R’ ’-’ ’-’ ’3X’ ’3X’ ”CS
 *  - 0x02: single measurement (0.1mm), needs: <ADDR>, <CS>
 *      +: ADDR 06 82”3X 3X 3X 2E 3X 3X 3X 3X”CS
 *      -: ADDR 06 82”’E’ ’R’ ’R’ ’-’ ’-’ ‘-‘’3X’ ’3X’ ”CS
 *  - 0x03: continuous measurement (1mm), needs: <ADDR>, <CS>
 *      +: ADDR 06 83” 3X 3X 3X 2E 3X 3X 3X”CS
 *      -: ADDR 06 83” ’E’ ’R’ ’R’ ’-’ ’-’ ’3X’ ’3X’”CS
 *  - 0x03: continuous measurement (0.1mm), needs: <ADDR>, <CS>
 *      +: ADDR 06 83” 3X 3X 3X 2E 3X 3X 3X 3X”CS
 *      -: ADDR 06 83” ’E’ ’R’ ’R’ ’-’ ’-’ ‘-‘’3X’ ’3X’”CS
 *  - 0x04: read machine number <CS=FC>
 *  - 0x05: control laser, needs: <laser=00(off),01(on)>
 *      +: ADDR 06 85 01 CS
 *      -: ADDR 06 85 00 CS
 *  - 0x06: single measurement, store in cache <CS=FA>
 *  - 0x07: read cache, needs: <ADDR>
 * }
 * 
 * -- Reply Message
 * <ADDR> <GRP> (<CMD> | 0x80) <CS>
 * <ADDR> [<GRP> | 0x80] (<CMD> | 0x80) <ERR> <CS>
 * 
 */
#define ADDRESS_BROADCAST (char)0xFA
#define DEVICE_ADDRESS (char)0x80  // default address
#define CMD_GRP_CNTRL (char)0x04
#define CMD_SET_RANGE (char)0x09
#define CMD_SET_RES (char)0x0C
#define CMD_GRP_READ (char)0x06
#define CMD_SET_LASER (char)0x05
#define CMD_READ_SINGLE (char)0x02


using namespace laser;

uint8_t Laser::calculate_checksum(char *message, uint8_t length)
{
    // sum of all bytes (and %256 cuz uint8_t)
    uint8_t sum = 0;
    for (size_t i = 0; i < length; ++i)
    {
        sum += (uint8_t)message[i];
    }

    // two's complement
    return ~sum + 1;
}

uint8_t Laser::read_sensor(char *buffer)
{
    // wait for sensor
    while (!(sensor_serial.available()>0)) { delay(20); };

    if (debugging)
        Serial.println("sensor available");

    delay(50);

    // read reply
    int i = 0;
    while (sensor_serial.available())
    {
        buffer[i]=sensor_serial.read();

        // detailed message
        if (debugging)
        {
            Serial.print((uint8_t)buffer[i], HEX); Serial.print(" ");
        }

        i++;
    }
    return i;
}


Laser::Laser(uint8_t rx_pin, uint8_t tx_pin, uint32_t baud)
  : sensor_serial({rx_pin, tx_pin})
{
    sensor_serial.begin(baud);
}


bool Laser::set_range(uint8_t range)
{
    char buff[5] = {
        ADDRESS_BROADCAST,
        CMD_GRP_CNTRL,
        CMD_SET_RANGE,
        (char)range,
        0
    };
    buff[4] = calculate_checksum(buff, 4);

    // send command to sensor
    sensor_serial.print(buff);

    if (debugging)
        Serial.println("message sent");

    // wait for sensor reply
    char read_buff[16];
    read_sensor(read_buff);

    if (debugging)
        Serial.println(read_buff[1], HEX);

    // check for success
    return read_buff[1] == 0x04;
}


bool Laser::set_resolution(bool resolution)
{
    char buff[5] = {
        ADDRESS_BROADCAST,
        CMD_GRP_CNTRL,
        CMD_SET_RES,
        (char)(resolution ? 0x02 : 0x01),
        (char)(resolution ? 0xF4 : 0xF5)  // don't compute CS for simple stuff
    };

    // send command to sensor
    sensor_serial.print(buff);

    // wait for sensor reply
    char read_buff[16];
    read_sensor(read_buff);

    // check for success
    return read_buff[1] == 0x04;
}


bool Laser::set_laser(bool state)
{
    char buff[5] = {
        DEVICE_ADDRESS,
        CMD_GRP_READ,
        CMD_SET_LASER,
        (char)state,
        (char)(state ? 0x74 : 0x75)  // don't compute CS for simple stuff
    };
    // Serial.println((uint8_t)buff[3], HEX);
    // Serial.println((uint8_t)buff[4], HEX);

    // send command to sensor
    sensor_serial.print(buff);

    // wait for sensor reply
    char read_buff[16];
    read_sensor(read_buff);

    // check for success
    return read_buff[3];
}


double Laser::measure()
{
       char buff[4] = {
        DEVICE_ADDRESS,
        CMD_GRP_READ,
        CMD_READ_SINGLE,
        0
    };
    buff[3] = calculate_checksum(buff, 3);

    // send command to sensor
    sensor_serial.print(buff);

    // wait for sensor reply
    char read_buff[16];
    uint8_t message_length = read_sensor(read_buff);

    if(read_buff[3]=='E' && read_buff[4]=='R' && read_buff[5]=='R')
    {
        if (debugging)
            Serial.println("Error reading sensor");

        return -1;
    }

    // convert from ascii 3x 3x 3x . 3x 3x 3x (3x) to normal
    double out = (
        100 * (read_buff[3] - 0x30) +
         10 * (read_buff[4] - 0x30) +
          1 * (read_buff[5] - 0x30) +
        // decimal point
         .1 * (read_buff[7] - 0x30) +
        .01 * (read_buff[8] - 0x30) +
       .001 * (read_buff[9] - 0x30)
    );

    if (debugging)
    {
        Serial.print("number: ");
        Serial.print((read_buff[3] - 0x30), DEC);
        Serial.print((read_buff[4] - 0x30), DEC);
        Serial.print((read_buff[5] - 0x30), DEC);
        Serial.print(".");
        Serial.print((read_buff[7] - 0x30), DEC);
        Serial.print((read_buff[8] - 0x30), DEC);
        Serial.println((read_buff[9] - 0x30), DEC);
    }

    // check if 1mm or 0.1mm precision
    if (message_length == 12)
    {
        out += 0.0001 * (read_buff[10] - 0x30);
    }

    return out;
}


// SoftwareSerial mySerial(2,3);//Define software serial, 3 is TX, 2 is RX
// char buff[4]={0x80,0x06,0x03,0x77};
// unsigned char data[11]={0};
//
// void setup()
// {
//  Serial.begin(115200);
//  mySerial.begin(9600);
// }
//
// void loop()
// {
//   mySerial.print(buff);
//   while(1)
//   {
//     if(mySerial.available()>0)//Determine whether there is data to read on the serial 
//     {
//       delay(50);
//       for(int i=0;i<11;i++)
//       {
//         data[i]=mySerial.read();
//       }
//       unsigned char Check=0;
//       for(int i=0;i<10;i++)
//       {
//         Check=Check+data[i];
//       }
//       Check=~Check+1;
//       if(data[10]==Check)
//       {
//         if(data[3]=='E'&&data[4]=='R'&&data[5]=='R')
//         {
//           Serial.println("Out of range");
//         }
//         else
//         {
//           float distance=0;
//           distance=(data[3]-0x30)*100+(data[4]-0x30)*10+(data[5]-0x30)*1+(data[7]-0x30)*0.1+(data[8]-0x30)*0.01+(data[9]-0x30)*0.001;
//           Serial.print("Distance = ");
//           Serial.print(distance,3);
//           Serial.println(" M");
//         }
//       }
//       else
//       {
//         Serial.println("Invalid Data!");
//       }
//     }
//     delay(20);
//   }
// }