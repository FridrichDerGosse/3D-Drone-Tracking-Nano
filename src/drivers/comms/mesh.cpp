#include "mesh.hpp"

void mesh::print_payload(payload_t payload)
{
    for (int i = 0; i < STRING_SIZE; i++)
    {
        byte b = payload.data[i];

        // check for null byte
        if (b == 0)
            break;

        Serial.print((char)b);
        // Serial.print(",");
    }
}

void mesh::payload_to_string(payload_t *payload, char *string)
{
    for (int i = 0; i < STRING_SIZE; i++)
    {
        byte b = payload->data[i];
        string[i] = (char)b;

        // check for null byte
        if (b == 0)
            break;
    }

    // Serial.print("payload: "); print_payload(*payload); Serial.println("\n");
    // Serial.print("converted to: "); Serial.println(string);
}


void mesh::string_to_payload(char *string, payload_t *payload)
{
    for (uint16_t i = 0; i < payload_size; i++)
    {
        char c = string[i];
        payload->data[i] = c;

        // check for null byte
        if (c == 0)
            break;
    }
}