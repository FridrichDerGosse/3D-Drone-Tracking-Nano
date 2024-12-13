#include <Arduino.h>
#include <SoftwareSerial.h>
#include "comms/debugging.h"
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

char laser::sensor_buff[16] = {0};

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

uint8_t Laser::read_sensor(char *buffer, int timeout)
{
    // wait for sensor
    while (!(sensor_serial.available()>0))
    {
        if (timeout <= 0)
        {
            #ifdef LASER_DEBUGGING
            Serial.print(F("failed to read sensor"));
            #endif

            return -1;
        }

        delay(20);
        timeout -= 20;
    };

    #ifdef LASER_DEBUGGING
    Serial.println(F("sensor available"));
    #endif

    delay(50);

    // read reply
    int i = 0;
    while (sensor_serial.available())
    {
        buffer[i]=sensor_serial.read();

        // detailed message
        #ifdef LASER_DEBUGGING
        Serial.print((uint8_t)buffer[i], HEX); Serial.print(" ");
        #endif

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
    sensor_buff[0] = ADDRESS_BROADCAST;
    sensor_buff[1] = CMD_GRP_CNTRL;
    sensor_buff[2] = CMD_SET_RANGE;
    sensor_buff[3] = (char)range;
    sensor_buff[4] = calculate_checksum(sensor_buff, 4);
    sensor_buff[5] = '\0';

    // send command to sensor
    sensor_serial.print(sensor_buff);

    #ifdef LASER_DEBUGGING
    Serial.println(F("message sent"));
    #endif

    // wait for sensor reply
    if (!read_sensor(sensor_buff))
        return false;

    #ifdef LASER_DEBUGGING
    Serial.println(sensor_buff[1], HEX);
    #endif

    // check for success
    return sensor_buff[1] == 0x04;
}


bool Laser::set_resolution(bool resolution)
{
    sensor_buff[0] = ADDRESS_BROADCAST;
    sensor_buff[1] = CMD_GRP_CNTRL;
    sensor_buff[2] = CMD_SET_RES;
    sensor_buff[3] = (char)(resolution ? 0x02 : 0x01);
    sensor_buff[4] = (char)(resolution ? 0xF4 : 0xF5);  // don't compute CS for simple stuff
    sensor_buff[5] = '\0';

    // send command to sensor
    sensor_serial.print(sensor_buff);

    // wait for sensor reply
    if (!read_sensor(sensor_buff))
        return false;

    // check for success
    return sensor_buff[1] == 0x04;
}


bool Laser::set_laser(bool state)
{
    sensor_buff[0] = DEVICE_ADDRESS;
    sensor_buff[1] = CMD_GRP_READ;
    sensor_buff[2] = CMD_SET_LASER;
    sensor_buff[3] = (char)state;
    sensor_buff[4] = (char)(state ? 0x74 : 0x75);  // don't compute CS for simple stuff
    sensor_buff[5] = '\0';

    // send command to sensor
    sensor_serial.print(sensor_buff);

    // wait for sensor reply
    if (!read_sensor(sensor_buff))
        return false;

    // check for success
    return sensor_buff[3];
}


float Laser::measure()
{
    sensor_buff[0] = DEVICE_ADDRESS;
    sensor_buff[1] = CMD_GRP_READ;
    sensor_buff[2] = CMD_READ_SINGLE;
    sensor_buff[3] = calculate_checksum(sensor_buff, 3);
    sensor_buff[4] = '\0';

    // send command to sensor
    sensor_serial.print(sensor_buff);

    // wait for sensor reply
    int message_length = read_sensor(sensor_buff);

    if (message_length <= 0)
        return -1;

    if(sensor_buff[3]=='E' && sensor_buff[4]=='R' && sensor_buff[5]=='R')
    {
        #ifdef LASER_DEBUGGING
        Serial.println(F("Error reading sensor"));
        #endif

        return -1;
    }

    // convert from ascii 3x 3x 3x . 3x 3x 3x (3x) to normal
    float out = (
        100 * (sensor_buff[3] - 0x30) +
         10 * (sensor_buff[4] - 0x30) +
          1 * (sensor_buff[5] - 0x30) +
        // decimal point
         .1 * (sensor_buff[7] - 0x30) +
        .01 * (sensor_buff[8] - 0x30) +
       .001 * (sensor_buff[9] - 0x30)
    );

    #ifdef LASER_DEBUGGING
    Serial.print(F("number: "));
    Serial.print((sensor_buff[3] - 0x30), DEC);
    Serial.print((sensor_buff[4] - 0x30), DEC);
    Serial.print((sensor_buff[5] - 0x30), DEC);
    Serial.print(".");
    Serial.print((sensor_buff[7] - 0x30), DEC);
    Serial.print((sensor_buff[8] - 0x30), DEC);
    Serial.println((sensor_buff[9] - 0x30), DEC);
    #endif

    // check if 1mm or 0.1mm precision
    if (message_length == 12)
    {
        out += 0.0001 * (sensor_buff[10] - 0x30);
    }

    return out;
}
