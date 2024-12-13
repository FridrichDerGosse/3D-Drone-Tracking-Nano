#include "comms/debugging.h"
#include "message_buffer.hpp"


MessageBuffer::MessageBuffer()
{
    rw_pos = 0;
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
        #ifdef DEBUGGING
        Serial.println(F("buffer overflow"));
        #endif
        return false;
    }

    // copy to buffer
    strncpy(messages[rw_pos & W_POS], message, STRING_SIZE);

    // +1 (first 4 bits)
    rw_pos = (((rw_pos & W_POS) + 1) & W_POS) | (rw_pos & ~W_POS);

    n_elements++;
    if ((rw_pos & W_POS) >= BUFFER_SIZE)
    {
        // reset write_pos
        rw_pos &= ~W_POS;
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
    strncpy(message, messages[(rw_pos & R_POS) >> 4], STRING_SIZE);

    // +1 (last 4 bits)
    rw_pos = (((rw_pos & R_POS) + (1 << 4)) & R_POS) | (rw_pos & ~R_POS);

    n_elements--;
    if (((rw_pos | R_POS) >> 4) >= BUFFER_SIZE)
    {
        // reset read_pos
        rw_pos &= ~R_POS;
    }

    return true;
}
