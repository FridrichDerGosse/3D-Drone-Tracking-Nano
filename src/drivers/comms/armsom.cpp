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

    uint8_t current_timeout = 0;
    while (current_timeout * 5 < timeout)
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
        current_timeout += 1;
    }

    return buffer->length() > 0;
}


#ifdef ARMSOM_FORWARD_DEBUGGING
char armsom::debug_buffer[STRING_SIZE];

void armsom::debug_start()
{
    Serial.print(F("{\"ctrl\":4,\"content\":\""));
}
void armsom::debug_end()
{
    Serial.print("\"}"); Serial.print('\0');
}

void armsom::debug(const char* message)
{
    // convert to string so " is being converted to \"
    snprintf(
        debug_buffer,
        STRING_SIZE,
        "{\"ctrl\":4,\"content\":\"%s\"}",
        message
    );

    // encapsulate to json and send
    Serial.print(debug_buffer); Serial.print('\0');
}

void armsom::debug(char c)
{
    // encapsulate to json and send
    debug_start();
    Serial.print(c);
    debug_end();
}

void armsom::debug(int number)
{
    // encapsulate to json and send
    debug_start();
    Serial.print(number);
    debug_end();
}
#endif
