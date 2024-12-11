#include "debugging.h"
#include "armsom.hpp"



bool armsom::write_string(String data)
{
    // write data and append null terminator to it
    Serial.print(data); Serial.print('\0');
    return true;
}


bool armsom::read_string(String *buffer, unsigned int timeout)
{
    #ifdef ARMSOM_DEBUGGING
    Serial.println(F("reading data... "));
    #endif

    unsigned int current_timeout = 0;
    while (current_timeout < timeout)
    {
        if (Serial.available())
        {
            current_timeout = 0;

            char tmp = (char)Serial.read();

            #ifdef ARMSOM_DEBUGGING
            Serial.print(F("read \"")); Serial.print(tmp); Serial.println("\"");
            #endif

            if (tmp == '\0' && buffer->length() > 1)
                return true;

            buffer->concat(tmp);

            continue;
        }

        #ifdef ARMSOM_DEBUGGING
        Serial.print(F("nothing available, timeout=")); Serial.println(current_timeout);
        #endif

        delay(5);
        current_timeout += 5;
    }

    return buffer->length() > 0;
}
