/**
 * @file server.cpp
 * @author Nilusink
 * @brief main Master Node program
 * @date 2024-12-10
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#include <Arduino.h>
#include <ArduinoJson.h>
#include "drivers/fan.hpp"
#include "drivers/laser.hpp"
#include "drivers/comms/armsom.hpp"
#include "drivers/comms/server.hpp"


/**
 * control messages
 * 	* ctrl=0: comm start (clients are both connected)
 *  * ctrl=1: ping (ping request)
 * 	* ctrl=2: pong (ping reply)
 *  * ctrl=3: RF message forward
 */


RF24 r(9, 10);
RF24Network n(r);
RF24Mesh m(r, n);
mesh::Server server(&r, &n, &m);

laser::Laser tof(2, 3, 9600);


void setup()
{
	// fan setup
	fan::setup();
	fan::off();

	// serial comms
	Serial.begin(9600);
	while (!Serial);
	Serial.flush();

	// mesh server
	server.debugging = false;
	server.init();
	server.start();

	// read all data sent bevore starting
	while (Serial.available()) { Serial.read(); }
}


JsonDocument json_input;
JsonDocument json_reply;
bool sent_clients_connected = false;
char net_message_buffer[STRING_SIZE];
void loop()
{
	server.update();

	// read client messages (and forward them after start)
	while (server.available())  // prioritize RF comms over serial
	{
		// make sure regulare updates are called
		server.update();

		// read available message and continue on fail
		if (!server.get_received_message(net_message_buffer))
			continue;

		// read available message and continue if not all clients are connected
		if (!server.clients_connected())
			continue;

		// try to convert to json
		DeserializationError error = deserializeJson(json_input, net_message_buffer);

		if (error)  // failed to convert
		{
			// ignore message and continue
			continue;
		}
		else  // successfully converted to json
		{
			json_reply["ctrl"] = 3;  // set message type to RF forward
			serializeJson(json_reply, net_message_buffer);
			armsom::write_string(net_message_buffer);
		}
	}

	// only start serial comms after both clients were connected (doesn't work otherwise)
	if (!server.clients_connected())
		return;

	// if it hasn't already been done, send "clients connected" confirmation to server
	if (!sent_clients_connected)
	{
		armsom::write_string("{\"ctrl\": 0}");
		sent_clients_connected = true;
	}

	// handle serial comms
	if (Serial.available())
	{
		// reset reply message content
		json_reply.clear();

		// receive message
		String buffer;
		armsom::read_string(&buffer);

		if (buffer.length() < 1)
		{
			if (armsom::debugging)
				Serial.println("buffer length below 2");

			return;
		}

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

				// measure twice bc idk how but he alwys delayed
				tof.measure();
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

			case 3:  // set stuff
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

					case 3:
					{
						// set fan speed
						fan::set_speed((uint8_t)obj["value"]);

						// can't fail, will always be true
						json_reply["ack"] = true;
						json_reply["value_echo"] = (uint8_t)obj["value"];
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
		json_reply["to"] = obj.containsKey("id") ? obj["id"] : -1;
		serializeJson(json_reply, buffer);
		armsom::write_string(buffer);
		Serial.flush();
	}
}
