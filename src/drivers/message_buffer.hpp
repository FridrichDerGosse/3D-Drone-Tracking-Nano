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


// very limited ram
#define BUFFER_SIZE 2
#define STRING_SIZE 64


#define W_POS 0b00001111
#define R_POS 0b11110000

class MessageBuffer
{
    private:
        uint8_t n_elements = 0;

        // represent read and write pos as one 8-bit integer (4 bit each)
        uint8_t rw_pos;  // first 4 write, last 4 read (only saves 1 byte why tf did I do this)

        char messages[BUFFER_SIZE][STRING_SIZE];

    public:
        MessageBuffer();

        bool available();

        bool add_message(char *message);

        /**
         * @brief read messsage and delete from buffer (move read pos)
         * 
         * @param message write target
         * @return true read success
         * @return false read fail
         */
        bool read(char *message);
};
