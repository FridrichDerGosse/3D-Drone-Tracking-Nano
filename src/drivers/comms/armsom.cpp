#include "armsom.hpp"



bool armsom::write_string(String data)
{
    // Serial.print("got: \"");
    Serial.print(data);
    // Serial.println("\"");
    return true;
}


bool armsom::read_string(String *buffer)
{
    while (Serial.available())
    {
        buffer->concat((char)Serial.read());
    }
    return true;
}
