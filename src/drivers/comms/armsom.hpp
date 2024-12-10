/**
 * @file armsom.hpp
 * @author Nilusink
 * @brief communication to the armsom computer
 * @date 2024-12-03
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#include <Arduino.h>


namespace armsom
{
    const bool debugging = false;

    bool write_string(String data);

    bool read_string(String *buffer, unsigned int timeout = 50);
}
