#include "message_buffer.hpp"


MessageBuffer::MessageBuffer()
{
    write_pos = 0;
    read_pos = 0;
}


bool MessageBuffer::available()
{
    return n_elements > 0;
}


bool MessageBuffer::add_message(char* message)
{
//     Serial.print("adding message: "); Serial.println(message);
    // check for buffer overflow
    if (n_elements >= BUFFER_SIZE)
    {
        Serial.println("buffer overflow");
        return false;
    }

    // copy to buffer
    strncpy(messages[write_pos], message, STRING_SIZE);
    write_pos++;
    n_elements++;
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

    read_pos++;
    n_elements--;
    if (read_pos >= BUFFER_SIZE)
    {
        read_pos = 0;
    }

    return true;
}
