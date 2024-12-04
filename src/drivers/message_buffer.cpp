#include "message_buffer.hpp"


template <uint8_t buffer_size, uint16_t string_size>
MessageBuffer<buffer_size, string_size>::MessageBuffer()
{
    write_pos = 0;
    read_pos = 0;
}


template <uint8_t buffer_size, uint16_t string_size>
bool MessageBuffer<buffer_size, string_size>::available()
{
    return write_pos - read_pos > 0;
}


template <uint8_t buffer_size, uint16_t string_size>
bool MessageBuffer<buffer_size, string_size>::add_message(const char* message)
{
    // check for buffer overflow
    if (write_pos < read_pos)
        return false;

    // copy to buffer
    strncpy(message[write_pos], message, string_size);
    write_pos++;

    if (write_pos >= buffer_size)
    {
        write_pos = 0;
    }

    // Success
    return true;
}


template <uint8_t buffer_size, uint16_t string_size>
bool MessageBuffer<buffer_size, string_size>::read(char* message)
{
    // check if anything is available
    if (!available())
        return false;
    
    // copy message to output
    strncpy(message, messages[read_pos], string_size);

    return true;
}
