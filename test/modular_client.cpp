/**
 * @brief armsom test
 * 
 */
#include "drivers/comms/mesh.hpp"

mesh::Client client(9, 10, 1);

void setup()
{
    Serial.begin(9600);
    Serial.println("initializing");

    client.init();

    Serial.println("trying to connect");
    
    if (!client.connect())
    {
        Serial.println("nRF-24 hardware error!");

        for (;;);
    }

    Serial.println("init done");
}


uint16_t n = 0;
uint32_t displayTimer = 0;
void loop()
{
    // client.update();

    // if (millis() - displayTimer >= 1000)
    // {
    //     displayTimer = millis();

    //     client.send({displayTimer, n});
    // }
}