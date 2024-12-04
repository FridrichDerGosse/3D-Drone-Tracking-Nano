#include "message_buffer.hpp"


MessageBuffer::MessageBuffer()
{
    write_pos = 0;
    read_pos = 0;
}


bool MessageBuffer::available()
{
    return write_pos - read_pos > 0;
}


bool MessageBuffer::add_message(const char* message)
{
    Serial.print("adding message: "); Serial.print(message);
    // check for buffer overflow
    if (write_pos < read_pos)
        return false;

    // copy to buffer
    strncpy(messages[write_pos], message, STRING_SIZE);
    write_pos++;

    if (write_pos >= BUFFER_SIZE)
    {
        write_pos = 0;
    }

    // Success
    return true;
}


bool MessageBuffer::read(char* message)
{
    // check if anything is available
    if (!available())
        return false;
    
    // copy message to output
    strncpy(message, messages[read_pos], STRING_SIZE);

    return true;
}
