/**
 * @file fan.hpp
 * @author Nilusink
 * @brief simple fan driver
 * @date 2024-11-30
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#pragma once
#include <Arduino.h>


#define FAN_CORRECTION_PIN A0
#define FAN_PWM_PIN 6


namespace fan
{
    /**
     * @brief performs initial setup for fan control
     * 
     */
    void setup();

    /**
     * @brief change fan speed
     * 
     * @param speed 0-255
     */
    void set_speed(uint8_t speed);

    /**
     * @brief turn fan off
     * 
     */
    void off();

    /**
     * @brief turn fan to full power
     * 
     */
    void full();
}