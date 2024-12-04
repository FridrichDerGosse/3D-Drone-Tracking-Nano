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
				if (debugging)
					Serial.println(F("Could not connect to network.\nretrying..."));
	
			} while (mesh->renewAddress() == MESH_DEFAULT_ADDRESS);
		}
		else
		{
			if (debugging)
				Serial.println(F("Radio hardware not responding."));
	
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

		if (debugging)
			Serial.print("Got ");

		payload_t payload;
		switch (header.type)
		{
		// Display the incoming millis() values from the sensor nodes
		case 'S':
			network->read(header, &payload, payload_size);
			if (debugging)
			{
				Serial.print("data: ");
				Serial.print(payload);
				Serial.print(" from RF24Network address 0");
				Serial.println(header.from_node, OCT);
			}
			break;

		default:
			network->read(header, 0, 0);
		
			if (debugging)
				Serial.println(header.type);
		
			break;
		}
	}
}

bool Client::send(payload_t payload, bool renew, uint8_t target)
{
	if (debugging)
	{
		Serial.print("sending payload: "); Serial.println(payload);
	}

	// Send an 'M' type message containing the current millis()
	if (!mesh->write(&payload, 'S', payload_size, target))
	{

		// If a write fails, check connectivity to the mesh network
		if (!mesh->checkConnection())
		{
			if (renew)
			{
				// refresh the network address
				if (debugging)
					Serial.println("Renewing Address");
	
				if (mesh->renewAddress() == MESH_DEFAULT_ADDRESS)
				{
					// If address renewal fails, reconfigure the radio and restart the mesh
					// This allows recovery from most if not all radio errors
					mesh->begin();
				}
			}
			else
			{
				if (debugging)
					Serial.println("Send fail, Test fail");
			}
			return false;
		}
		else
		{
			if (debugging)
				Serial.println("Send fail, Test OK");
		
			return false;
		}
	}
	else
	{
		if (debugging)
			Serial.println("Send OK");

		return true;
	}
}


