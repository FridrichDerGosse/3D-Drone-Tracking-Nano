#include "debugging.h"
#include "server.hpp"


using namespace mesh;

int mesh::connected_clilents[2][2] = {
	{1, 0},  // id 1 not connected
	{2, 0}   // id 2 not connected
};


// mesh master
void Server::try_set_connected(uint8_t node_id)
{
	// if node in list, set it's connection status to true
	for (int i; i < 2; i++)
	{
		if (connected_clilents[i][0] == node_id)
			connected_clilents[i][1] = 1;
	}
}

bool Server::clients_connected()
{
	// check if both clients were connected
	return connected_clilents[0][1] && connected_clilents[1][1];
}

Server::Server(RF24 *radio, RF24Network *network, RF24Mesh *mesh)
	: radio(radio), network(network), mesh(mesh)
{}

void Server::init()
{
	#ifdef COMMS_DEBUGGING
	Serial.println(F("initializing"));
	#endif

	// set to master
	mesh->setNodeID(0);

	// set emmision strengh
	radio->begin();
	radio->setPALevel(RF24_PA_MAX, 0);

	#ifdef COMMS_DEBUGGING
	Serial.println(F("set power level"));
	#endif
}

bool Server::start()
{
	#ifdef COMMS_DEBUGGING
	Serial.println(F("starting"));
	#endif

	if (!mesh->begin())
	{
		#ifdef COMMS_DEBUGGING
		Serial.println(F("Radio hardware not responding."));
		#endif

		return false;
	}
	#ifdef COMMS_DEBUGGING
	Serial.println(F("server started"));
	#endif

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
	while (network->available())
	{
		#ifdef COMMS_DEBUGGING
		Serial.println(F("net available"));
		#endif

		RF24NetworkHeader header;
		network->peek(header);

		#ifdef COMMS_DEBUGGING
		Serial.println(F("Got "));
		#endif

		payload_t payload = {0};
		switch (header.type)
		{
			// Display the incoming millis() values from the sensor nodes
			case 'S':
				network->read(header, &payload, payload_size);

				#ifdef COMMS_DEBUGGING
				Serial.print(header.id);
				Serial.print(F(", data: \""));
				mesh::print_payload(payload);
				Serial.print(F("\" from RF24Network address 0"));
				Serial.print(header.from_node, OCT);
				Serial.print(F(", type: "));
				Serial.println((char)header.type);
				#endif

				// set client[node_id] to connected
				if (!clients_connected())
					try_set_connected(header.from_node);
		
				// append to queue
				char buffer[STRING_SIZE];
				mesh::payload_to_string(&payload, buffer);
				receive_buffer.add_message(buffer);

				break;

			default:
				network->read(header, 0, 0);

				#ifdef COMMS_DEBUGGING
				Serial.print(F("unknown header type: ")); Serial.print((char)header.type);
				Serial.print(F(" (")); Serial.print((uint8_t)header.type); Serial.println(")");
				#endif

				break;
		}

		// update network again in case of multiple messages
		mesh->update();
		mesh->DHCP();
	}
}

bool Server::send(payload_t payload, uint8_t target)
{
	// RF24NetworkHeader header(mesh->getAddress(target), OCT);
	if (!mesh->write(&payload, 'S', payload_size, target))
	{
		#ifdef COMMS_DEBUGGING
		Serial.println(F("send fail"));
		#endif

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
	return receive_buffer.read(buffer);
}