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

// JsonObject obj;

String buffer;
float sensor_val;
JsonDocument obj;
DeserializationError error;
mesh::payload_t net_message_payload;
bool sent_clients_connected = false;
char net_message_buffer[STRING_SIZE];
void loop()
{
	server.update();

	// read client messages (and forward them after start)
	while (server.available())  // prioritize RF comms over serial
	{
		// read available message and continue on fail
		if (!server.get_received_message(net_message_buffer))
			continue;

		// insert ctrl=3 into the json object
		buffer = net_message_buffer;
		snprintf(
			net_message_buffer,
			STRING_SIZE,
			"{\"ctrl\":3,\"data\":%s}",
			buffer.c_str()
		);

		// send json object to armsom
		armsom::write_string(net_message_buffer);
		Serial.flush();

		// make sure regulare updates are called
		server.update();
	}

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
		deserializeJson(obj, buffer);
		// obj = json_doc.as<JsonObject>();

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
					// convert message into payload
					mesh::string_to_payload(obj["data"], &net_message_payload);

					// send message to net and reply to armsom
					snprintf(
						net_message_buffer,
						STRING_SIZE,
						"{\"type\": 0, \"ack\": %d, \"to\": %d}",
						server.send(net_message_payload, obj["target"]),
						reply_to
					);
					break;
				}

				case 2:  // get laser data
				{
					// measure twice bc idk how but he alwys delayed
					tof.measure();
					sensor_val = tof.measure();

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

/**
before main:
RAM:   [========  ]  76.2% (used 1560 bytes from 2048 bytes)
Flash: [=====     ]  49.7% (used 15270 bytes from 30720 bytes)

after:
RAM:   [========= ]  93.5% (used 1914 bytes from 2048 bytes)
Flash: [========  ]  79.2% (used 24342 bytes from 30720 bytes)

smaller net message buffer:
RAM:   [========  ]  77.8% (used 1594 bytes from 2048 bytes)
Flash: [========  ]  78.5% (used 24118 bytes from 30720 bytes)

message_buffer optimizations:
RAM:   [========  ]  77.8% (used 1593 bytes from 2048 bytes)
Flash: [========  ]  78.5% (used 24124 bytes from 30720 bytes)

laser opt: (more RAM but takes (close to) no dynamic RAM)
RAM:   [========  ]  78.4% (used 1605 bytes from 2048 bytes)
Flash: [========  ]  77.9% (used 23946 bytes from 30720 bytes)

server opt:
RAM:   [========  ]  78.2% (used 1601 bytes from 2048 bytes)
Flash: [========  ]  77.8% (used 23912 bytes from 30720 bytes)

Mesh opt:
RAM:   [========  ]  78.0% (used 1597 bytes from 2048 bytes)
Flash: [========  ]  77.9% (used 23918 bytes from 30720 bytes)

no armsom debugging:
RAM:   [=======   ]  72.2% (used 1479 bytes from 2048 bytes)
Flash: [========  ]  76.5% (used 23502 bytes from 30720 bytes)
 */
