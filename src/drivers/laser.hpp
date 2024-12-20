/**
 * @file laser.hpp
 * @author Nilusink
 * @brief measures distances using a TOF laser
 * @date 2024-12-04
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#include <Arduino.h>
#include <SoftwareSerial.h>


namespace laser
{
    extern char sensor_buff[16];

    class Laser
    {
        private:
            SoftwareSerial sensor_serial;

        protected:
            uint8_t calculate_checksum(char *message, uint8_t length);

            /**
             * @brief reads a sensor string
             * 
             * @param buffer write target
             * @param timeout max time to wait for answer
             * @return uint8_t length, negative: error
             */
            uint8_t read_sensor(char *buffer, int timeout=500);

            // float ascii_to_value(char *message)

        public:
            Laser(uint8_t rx_pin, uint8_t tx_pin, uint32_t baud);

            /**
             * @brief sensor range
             * 
             * @param range must be 5, 10, 30, 50 or 80
             * @return true success
             * @return false fail
             */
            bool set_range(uint8_t range);

            /**
             * @brief set the sensor resolution
             * 
             * @param resolution 0: 1mm, 1: 0.1mm
             * @return true success
             * @return false fail
             */
            bool set_resolution(bool resolution);

            /**
             * @brief turn laser on / off
             * 
             * @param state 0: off, 1: on
             * @return true success
             * @return false fail
             */
            bool set_laser(bool state);

            /**
             * @brief measure distances with the laser
             * 
             * @return double distance in m, negative values are fail
             */
            float measure();
    };
}
