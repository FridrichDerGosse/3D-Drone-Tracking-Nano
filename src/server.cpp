#include <Arduino.h>
#include <ArduinoJson.h>
#include "drivers/comms/armsom.hpp"
#include "drivers/laser.hpp"
#include "drivers/fan.hpp"


/**
 * 
 * from armsom:
 * {
 *      "type": 0=send, 1=get_received, 2=get_laser
 * }
 * 
 * to armsom:
 * {
 *      "type": 0=ack, 1=network_data, 2=laser_data
 * }
 */

laser::Laser tof(2, 3, 9600);

void setup()
{
	// fan setup
	fan::setup();
	fan::off();

	// serial comms
	Serial.begin(9600);
	while (!Serial);

    // tof sensor settings
    Serial.print("setting range: "); Serial.println(tof.set_range(30));
    Serial.print("setting resolution: "); Serial.println(tof.set_resolution(1));

    // no clue why it doesn't work
    // for (int i=0; i < 3; i++)
    // {
    //     Serial.print("setting laser: "); Serial.println(tof.set_laser(0));
    //     delay(500);
    Serial.print("setting laser: "); Serial.println(tof.set_laser(1));
    //     delay(500);
    // }
}


JsonDocument json_input;
JsonDocument json_reply;
void loop()
{
    double distance = tof.measure();
    char buff[32];
    snprintf(buff, 31, "distance: %d mm", (int)(distance*1000));
    Serial.println(buff);

    delay(500);
    return;
	// armsom comms
	if (Serial.available())
	{
		Serial.println("SER: available");
		// turn fan on while receiving
		fan::full();

		// reset reply message content
		json_reply.clear();

		// receive message
		String buffer;
		armsom::read_string(&buffer);

		// convert message to json
		deserializeJson(json_input, buffer);
		// auto json_input = json::parse(buffer);

		JsonObject obj = json_input.as<JsonObject>();

		// handle message
		switch ((int)obj["type"])
		{
			case 0: // send
			{
				json_reply["type"] = 0;  // ack

				break;
			}

			case 1: // get_received
			{
				json_reply["type"] = 1;  // messatge data
                json_reply["available"] = 0;
			}

            case 2:
            {
                json_reply["type"] = 2;  // laser data
            }

			default:  // unknown message type
			{
				json_reply["type"] = 0; // ack
				json_reply["ack"] = 0;
				break;
			}
		}

		// reply to message
		serializeJson(json_reply, buffer);
		armsom::write_string(buffer);

		// delay(100);

		// turn fan back off
		fan::off();
	}
}
