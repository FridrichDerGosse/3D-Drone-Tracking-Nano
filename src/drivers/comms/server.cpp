#include "server.hpp"


using namespace mesh;


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
		if (debugging)
			Serial.println("net available");

		RF24NetworkHeader header;
		network->peek(header);

		if (debugging)
			Serial.println("Got ");

		payload_t payload = {0};
		switch (header.type)
		{
		// Display the incoming millis() values from the sensor nodes
		case 'S':
			network->read(header, &payload, payload_size);

			if (debugging)
			{
				Serial.print(header.id);
				Serial.print(", data: \"");
				mesh::print_payload(payload);
				Serial.print("\" from RF24Network address 0");
				Serial.print(header.from_node, OCT);
				Serial.print(", type: ");
				Serial.println((char)header.type);
			}

			// append to queue
			// if (!message_written)
			// {
			// 	message_written = true;
			// 	strncpy(last_message, payload.data, 64);
			// }
			char buffer[STRING_SIZE];
			mesh::payload_to_string(&payload, buffer);
			receive_buffer.add_message(buffer);

			break;

		default:
			network->read(header, 0, 0);

			if (debugging)
			{
				Serial.print("unknown header type: "); Serial.print((char)header.type);
				Serial.print(" ("); Serial.print((uint8_t)header.type); Serial.println(")");
			}

			break;
		}
	}
}

bool Server::send(payload_t payload, uint8_t target)
{
	// RF24NetworkHeader header(mesh->getAddress(target), OCT);
	if (!mesh->write(&payload, 'S', payload_size, target))
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