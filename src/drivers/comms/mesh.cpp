#include "mesh.hpp"

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
		case 'M':
			network->read(header, &payload, sizeof(payload));
			if (debugging)
			{
				Serial.print(payload.id);
				Serial.print(", data: ");
				Serial.print(payload.data);
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
	// Send an 'M' type message containing the current millis()
	if (!mesh->write(&payload, 'M', sizeof(payload), target))
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


// mesh master
Server::Server(RF24 *radio, RF24Network *network, RF24Mesh *mesh)
	: radio(radio), network(network), mesh(mesh)
{}

void Server::init()
{
	if (debugging)
		Serial.println("initializing");

	// set to master
	mesh->setNodeID(0);

	// set emmision strengh
	radio->begin();
	radio->setPALevel(RF24_PA_MAX, 0);

	if (debugging)
		Serial.println("set power level");
}

bool Server::start()
{
	if (debugging)
		Serial.println("starting");

	if (!mesh->begin())
	{
		// if mesh.begin() returns false for a master node, then radio.begin() returned false.
		if (debugging)
			Serial.println(F("Radio hardware not responding."));

		return false;
	}
	if (debugging)
		Serial.println("server started");

	return true;
}

void Server::update()
{
	// Call mesh.update to keep the network updated
	mesh->update();

	// In addition, keep the 'DHCP service' running on the master node so addresses will
	// be assigned to the sensor nodes
	mesh->DHCP();

	// Check for incoming data from the sensors
	if (network->available())
	{
		RF24NetworkHeader header;
		network->peek(header);

		if (debugging)
			Serial.print("Got ");

		payload_t payload;
		switch (header.type)
		{
		// Display the incoming millis() values from the sensor nodes
		case 'M':
			network->read(header, &payload, sizeof(payload));

			if (debugging)
			{
				Serial.print(header.id);
				Serial.print(", data: \"");
				Serial.print(payload.data);
				Serial.print("\" from RF24Network address 0");
				Serial.println(header.from_node, OCT);
			}

			// append to queue
			// if (!message_written)
			// {
			// 	message_written = true;
			// 	strncpy(last_message, payload.data, 64);
			// }
			receive_buffer.add_message(payload.data);

			break;

		default:
			network->read(header, 0, 0);

			if (debugging)
				Serial.println(header.type);

			break;
		}
	}
}

bool Server::send(payload_t payload, uint8_t target)
{
	RF24NetworkHeader header(mesh->getAddress(target), OCT);
	if (!network->write(header, &payload, sizeof(payload)))
	{
		if (debugging)
			Serial.println("send fail");

		return false;
	}
	return true;
}

bool Server::available()
{
	// return message_written;
	return receive_buffer.available();
}

bool Server::get_received_message(char *buffer)
{
	if (!available())
		return false;
	
	// copy buffer content
	// strncpy(buffer, last_message, 64);
	receive_buffer.read(buffer);

	return true;
}
