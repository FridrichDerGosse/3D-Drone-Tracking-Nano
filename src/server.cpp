#include <Arduino.h>
#include <ArduinoJson.h>
#include "drivers/comms/armsom.hpp"
#include "drivers/laser.hpp"
#include "drivers/fan.hpp"


/**
 * 
 * from armsom:
 * {
 *      "type": 0=send, 1=get_received, 2=get_laser, 3=set_laser
 * 
 * 		set_laser:
 * 			"target": 0=laser(on, occ), 1=resolution, 2=range
 * 			"value"
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
    // tof.set_range(80);
    // tof.set_resolution(1);
	// Serial.println(tof.measure());

	// turn laser on {"type": 2}
    // tof.set_laser(1);
}


JsonDocument json_input;
JsonDocument json_reply;
void loop()
{
	// for (;;)
	// {
	// 	Serial.println(tof.measure());
	// 	delay(500);
	// }
    // double distance = tof.measure();
    // char buff[32];
    // snprintf(buff, 31, "distance: %d mm", (int)(distance*1000));
    // Serial.println(buff);

    // delay(500);
    // return;
	// armsom comms

	// Serial.println(".");

	if (Serial.available())
	{
		// Serial.println("SER: available");
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
				json_reply["ack"] = 0;

				break;
			}

			case 1: // get_received
			{
				json_reply["type"] = 1;  // messatge data
                json_reply["available"] = 0;

				break;
			}

            case 2:  // get laser data
            {
                json_reply["type"] = 2;  // laser data

				double sensor_val = tof.measure();

				if (sensor_val > 0)
				{
					json_reply["valid"] = true;
					json_reply["distance"] = sensor_val;
				}
				else
				{
					json_reply["valid"] = false;
				}
				break;
            }

			case 3:  // set laser
			{
				json_reply["type"] = 0;

				// match target
				switch ((int)obj["target"])
				{
					case 0:
					{
						json_reply["ack"] = tof.set_laser((bool)obj["value"]);
						break;
					}

					case 1:
					{
						json_reply["ack"] = tof.set_resolution((bool)obj["value"]);
						break;
					}

					case 2:
					{
						json_reply["ack"] = tof.set_range((uint8_t)obj["value"]);
						break;
					}

					default:
						json_reply["ack"] = 0;
						break;
				}

				break;
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
