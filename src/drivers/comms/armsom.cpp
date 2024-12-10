#include "armsom.hpp"



bool armsom::write_string(String data)
{
    // Serial.print("got: \"");
    Serial.print(data); Serial.print('\0');
    // Serial.println("\"");
    return true;
}


bool armsom::read_string(String *buffer, unsigned int timeout)
{
    if (debugging)
        Serial.println("reading data... ");

    unsigned int current_timeout = 0;
    while (current_timeout < timeout)
    {
        if (Serial.available())
        {
            current_timeout = 0;

            char tmp = (char)Serial.read();

            if (debugging)
            {
                Serial.print("read \""); Serial.print(tmp); Serial.println("\"");
            }

            if (tmp == '\0' && buffer->length() > 1)
                return true;

            buffer->concat(tmp);

            continue;
        }

        if (debugging)
        {
            Serial.print("nothing available, timeout="); Serial.println(current_timeout);
        }

        delay(5);
        current_timeout += 5;
    }

    return buffer->length() > 0;
}
