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
#include "drivers/comms/debugging.h"


/**
 * control messages (to server)
 * 	* ctrl=0: comm start (clients are both connected)
 *  * ctrl=1: ping (ping request) REQUIRES: id
 * 	* ctrl=2: pong (ping reply) REQUIRES: to
 *  * ctrl=3: RF message forward REQUIRES: data
 *  * ctrl=4: debug message (print forward) REQUIRES: content
 * 
 * control messages (from server)
 * 	* ctrl=0: -
 *  * ctrl=1: ping (ping request)
 * 	* ctrl=2: pong (ping reply)
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
	if (!server.init() || !server.start())
	{
		// forward error to armsom
		#ifdef ARMSOM_FORWARD_DEBUGGING
		armsom::debug("nRF Hardware error!");
		#endif

		// wait for 5 seconds
		delay(5000);

		// retry setup
		setup();
	}

	// read all data sent bevore starting
	while (Serial.available()) { Serial.read(); }

	#ifdef ARMSOM_FORWARD_DEBUGGING
	armsom::debug("started");
	#endif
}
// before: 95.9%
// after json removal: 76.2%

// before debugging optimization: 68.8% RAM, 78.4% Flash
// after debugging optimization: 67.4% RAM, 73.8% Flash


String buffer;
JsonObject obj;
JsonDocument json_doc;
DeserializationError error;
bool sent_clients_connected = false;
char net_message_buffer[STRING_SIZE];
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
	return;

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

		// insert ctrl=3 into the jsono object
		buffer = net_message_buffer;
		snprintf(
			net_message_buffer,
			STRING_SIZE,
			"{\"ctrl\":3,\"data\":%s}",
			buffer.c_str()
		);

		// buffer = "{\"ctrl\": 3," ;
		// net_message_buffer[0] = ' ';
		// buffer.concat(net_message_buffer);

		// send json object to armsom
		armsom::write_string(net_message_buffer);
		Serial.flush();

		// // try to convert to json
		// error = deserializeJson(json_doc, net_message_buffer);

		// if (error)  // failed to convert
		// {
		// 	// ignore message and continue
		// 	continue;
		// }
		// else  // successfully converted to json
		// {
		// 	// set message type to RF forward and send
		// 	json_doc["ctrl"] = 3;
		// 	serializeJson(json_doc, net_message_buffer);
		// 	armsom::write_string(net_message_buffer);
		// }
	}

	return;

	// only start serial comms after both clients were connected (doesn't work otherwise)
	if (!server.clients_connected())
		return;

	// if it hasn't already been done, send "clients connected" confirmation to server
	if (!sent_clients_connected)
	{
		armsom::write_string(F("{\"ctrl\": 0}"));
		sent_clients_connected = true;
	}

	// handle serial comms
	if (Serial.available())
	{
		// receive message
		armsom::read_string(&buffer);

		if (buffer.length() < 1)
		{
			#ifdef ARMSOM_DEBUGGING
			Serial.println(F("buffer length below 2"));
			#endif

			return;
		}

		// convert message to json
		deserializeJson(json_doc, buffer);

		obj = json_doc.as<JsonObject>();

		// clear net buffer to be used as reply
		memset(net_message_buffer, '\0', STRING_SIZE);

		// get reply id
		int reply_to = obj.containsKey("id") ? obj["id"] : -1;

		// handle control messsage (use strings as reply cuz ram and shit)
		if (obj.containsKey("ctrl"))
		{
			switch ((int)obj["ctrl"])
			{
				case 1:  // ping
				{
					snprintf(
						net_message_buffer,
						STRING_SIZE-1,
						"{\"ctrl\": 2, \"time\": %d, \"to\": %d}",
						(int)obj["time"],
						reply_to
					);
				}
				default:
					break;
			}
		}
		else
		{
			// handle regular message
			switch ((int)obj["type"])
			{
				case 0: // send
				{
					mesh::payload_t initial_message;
					mesh::string_to_payload(obj["data"], &initial_message);

					snprintf(
						net_message_buffer,
						STRING_SIZE,
						"{\"type\": 0, \"ack\": %d, \"to\": %d}",
						server.send(initial_message, obj["target"]),
						reply_to
					);
					break;
				}

				case 2:  // get laser data
				{
					// measure twice bc idk how but he alwys delayed
					tof.measure();
					float sensor_val = tof.measure();

					if (sensor_val > 0)
					{
						snprintf(
							net_message_buffer,
							STRING_SIZE-1,
							"{\"type\": 2, \"valid\": 1, \"distance\": %d, \"to\": %d}",
							(int)sensor_val*1000,
							reply_to
						);
					}
					else
					{
						snprintf(
							net_message_buffer,
							STRING_SIZE,
							"{\"type\": 2, \"valid\": 0, \"to\": %d}",
							reply_to
						);
					}
					break;
				}

				case 3:  // set stuff
				{
					// match target
					bool status = false;
					switch ((int)obj["target"])
					{
						case 0:
						{
							status = tof.set_laser((bool)obj["value"]);
							break;
						}

						case 1:
						{
							status = tof.set_resolution((bool)obj["value"]);
							break;
						}

						case 2:
						{
							status = tof.set_range((uint8_t)obj["value"]);
							break;
						}

						case 3:
						{
							// set fan speed
							fan::set_speed((uint8_t)obj["value"]);

							// can't fail, will always be true
							status = true;
							break;
						}

						default:
							break;
					}

					snprintf(
						net_message_buffer,
						STRING_SIZE,
						"{\"type\": 0, \"ack\": %d, \"to\": %d}",
						status,
						reply_to
					);
					break;
				}

				default:  // unknown message type
				{
					snprintf(
						net_message_buffer,
						STRING_SIZE,
						"{\"type\": 0, \"ack\": 0, \"to\": %d}",
						reply_to
					);
					break;
				}
			}
		}

		// reply to message
		armsom::write_string(net_message_buffer);
		Serial.flush();
	}
}
