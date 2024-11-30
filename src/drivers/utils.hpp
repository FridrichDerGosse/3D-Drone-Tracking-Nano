/**
 * @file utils.hpp
 * @author Nilusink
 * @brief various useful functions
 * @date 2024-11-30
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#pragma once
#include <avr/wdt.h>


namespace utils
{
    // types
    typedef uint8_t pin_t;

    /**
     * @brief restarts the arduino by triggering the watchdog
     * 
     */
    void restart();
}
