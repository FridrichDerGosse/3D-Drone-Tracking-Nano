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
    unsigned int current_timeout = 0;
    while (current_timeout < timeout)
    {
        if (Serial.available())
        {
            current_timeout = 0;

            char tmp = (char)Serial.read();

            if (tmp == '\0')
                return true;

            buffer->concat(tmp);

            continue;
        }

        delay(5);
        current_timeout += 5;
    }

    return false;
}
