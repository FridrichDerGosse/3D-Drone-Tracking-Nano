/**
 * @file message_buffer.hpp
 * @author Nilusink
 * @brief temporary storage for messages
 * @date 2024-12-04
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#pragma once
#include <Arduino.h>


template <uint8_t buffer_size, uint16_t string_size>
class MessageBuffer
{
    private:
        uint8_t write_pos;
        uint8_t read_pos;

        char messages[buffer_size][string_size];

    public:
        MessageBuffer();

        bool available();

        bool add_message(const char *message);

        bool read(char *message);
};
