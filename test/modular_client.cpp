/**
 * @brief armsom test
 * 
 */
#include "drivers/comms/client.hpp"


RF24 r(9, 10);
RF24Network n(r);
RF24Mesh m(r, n);
mesh::Client client(&r, &n, &m, 1);


void setup()
{
    Serial.begin(9600);
    while (!Serial);

    Serial.println("initializing");
    client.init();
	client.debugging = true;

    Serial.println("trying to connect");
    if (!client.connect())
    {
        Serial.println("nRF-24 hardware error!");

        for (;;);
    }

    Serial.println("setup done");

    delay(100);
}


uint16_t counter = 0;
uint32_t displayTimer = 0;
void loop()
{
    client.update();

    if (millis() - displayTimer >= 500)
    {
        displayTimer = millis();
        mesh::payload_t initial_message;
        mesh::string_to_payload("{\"type\": 11}", &initial_message);
	    Serial.println(client.send(initial_message));
    }
    // {

    //     uint8_t fails = 0;
    //     while (!client.send({displayTimer, counter}, false))
    //     {
    //         Serial.println("failed");
    //         fails++;

    //         if (fails > 10)
    //         {
    //             Serial.println("trying to send with renew");
    //             delay(10);
    //             client.send({displayTimer, counter}, true);
    //             break;
    //         }
    //     }
    //     counter++;
    // }
}
