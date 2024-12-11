#include "debugging.h"
#include "client.hpp"

using namespace mesh;


// mesh client
Client::Client(RF24 *radio, RF24Network *network, RF24Mesh *mesh, uint8_t node_id)
	: node_id(node_id), radio(radio), network(network), mesh(mesh)
{
}

void Client::init()
{
	// setup mesh
	mesh->setNodeID(node_id);

	// set emmision strengh
	radio->begin();
	radio->setPALevel(RF24_PA_MAX, 0);
}

bool Client::connect(uint8_t retries)
{
	if (!mesh->begin())
	{
		if (radio->isChipConnected())
		{
			do
			{
				// mesh->renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
				#ifdef COMMS_DEBUGGING
				Serial.println(F("Could not connect to network.\nretrying..."));
				#endif

			} while (mesh->renewAddress() == MESH_DEFAULT_ADDRESS);
		}
		else
		{
			#ifdef COMMS_DEBUGGING
			Serial.println(F("Radio hardware not responding."));
			#endif
	
			return false;
		}
	}
	return true;
}

void Client::update()
{
	mesh->update();

	// check for available messages
	while (network->available())
	{
		RF24NetworkHeader header;
		network->peek(header);

		#ifdef COMMS_DEBUGGING
		Serial.print("Got ");
		#endif

		payload_t payload;
		switch (header.type)
		{
		// Display the incoming millis() values from the sensor nodes
		case 'S':
			network->read(header, &payload, payload_size);

			#ifdef COMMS_DEBUGGING
				Serial.print(F("data: "));
				mesh::print_payload(payload);
				Serial.print(F(" from RF24Network address 0"));
				Serial.println(header.from_node, OCT);
			#endif
			break;

		default:
			network->read(header, 0, 0);
		
			#ifdef COMMS_DEBUGGING
			Serial.println(header.type);
			#endif
		
			break;
		}
	}
}

bool Client::send(payload_t payload, bool renew, uint8_t target)
{
	#ifdef COMMS_DEBUGGING
	Serial.print(F("sending payload: ")); mesh::print_payload(payload);
	#endif

	// Send an 'M' type message containing the current millis()
	if (!mesh->write(&payload, 'S', payload_size, target))
	{

		// If a write fails, check connectivity to the mesh network
		if (!mesh->checkConnection())
		{
			if (renew)
			{
				// refresh the network address
				#ifdef COMMS_DEBUGGING
				Serial.println(F("Renewing Address"));
				#endif
	
				if (mesh->renewAddress() == MESH_DEFAULT_ADDRESS)
				{
					// If address renewal fails, reconfigure the radio and restart the mesh
					// This allows recovery from most if not all radio errors
					mesh->begin();
				}
			}
			else
			{
				#ifdef COMMS_DEBUGGING
				Serial.println(F("Send fail, Test fail"));
				#endif
			}
			return false;
		}
		else
		{
			#ifdef COMMS_DEBUGGING
			Serial.println(F("Send fail, Test OK"));
			#endif
		
			return false;
		}
	}
	else
	{
		#ifdef COMMS_DEBUGGING
		Serial.println(F("Send OK"));
		#endif

		return true;
	}
}


