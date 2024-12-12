#include <Arduino.h>
#include <ArduinoJson.h>
#include "drivers/comms/armsom.hpp"
#include "drivers/comms/server.hpp"
#include "drivers/fan.hpp"


RF24 r(9, 10);
RF24Network n(r);
RF24Mesh m(r, n);
mesh::Server server(&r, &n, &m);


void setup()
{
	// fan setup
	fan::setup();
	fan::off();

	// serial comms
	Serial.begin(9600);
	while (!Serial);

	// mesh server
	server.init();
	server.start();
}


bool clients_connected = false;
char net_message_buffer[STRING_SIZE];
JsonDocument json_input;
JsonDocument json_reply;
void loop()
{
	server.update();

	while (server.available())
	{
		server.get_received_message(net_message_buffer);
		Serial.print("client message: ");
		Serial.println(net_message_buffer);

		// clients_connected = true;
	}

	if (!clients_connected)
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
				
				// send message to server
				mesh::payload_t payload;
                strncpy(net_message_buffer, obj["data"], STRING_SIZE);
                mesh::string_to_payload(net_message_buffer, &payload);
				json_reply["ack"] = server.send(payload, obj["to"]);

				break;
			}

			case 1: // get_received
			{
				json_reply["type"] = 1;  // data

				if (!server.available())
				{
					json_reply["available"] = 0;
				}
				else
				{
					json_reply["available"] = 1;

					// add data to reply
					JsonArray data = json_reply["data"].to<JsonArray>();

					// add all received messages
					while (server.available())
					{
						server.get_received_message(net_message_buffer);
						data.add(net_message_buffer);
					}
				}
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
