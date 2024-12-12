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
#include "../message_buffer.hpp"


namespace armsom
{
    bool write_string(String data);

    bool read_string(String *buffer, unsigned int timeout = 50);

    // #ifdef ARMSOM_FORWARD_DEBUGGING
    extern char debug_buffer[STRING_SIZE];

    // #ifdef ARMSOM_FORWARD_DEBUGGING
    void debug_start();
    void debug_end();

    /**
     * @brief send debug message to armsom
     * 
     * @param message message to send
     */
    void debug(const char* message);
    void debug(char c);
    void debug(int number);
    // #endif
}
