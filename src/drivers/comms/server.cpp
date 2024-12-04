#include "server.hpp"


using namespace mesh;


// mesh master
Server::Server(RF24 *radio, RF24Network *network, RF24Mesh *mesh)
	: radio(radio), network(network), mesh(mesh)
{
}

void Server::init()
{
	// set to master
	mesh->setNodeID(0);

	// set emmision strengh
	radio->begin();
	radio->setPALevel(RF24_PA_MAX, 0);
}

bool Server::start()
{
	if (!mesh->begin())
	{
		// if mesh.begin() returns false for a master node, then radio.begin() returned false.
		if (debugging)
			Serial.println(F("Radio hardware not responding."));

		return false;
	}
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
				Serial.print(payload.id);
				Serial.print(", data: ");
				Serial.print(payload.data);
				Serial.print(" from RF24Network address 0");
				Serial.println(header.from_node, OCT);
			}

			// append to queue
			receive_buffer->add_message(payload.data);

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
	return receive_buffer->available();
}

bool Server::get_received_message(char *buffer)
{
	if (!available())
		return false;
	
	// copy buffer content
	receive_buffer->read(buffer);

	return true;
}
