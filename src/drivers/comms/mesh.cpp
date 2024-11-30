#include "mesh.hpp"

using namespace mesh;

Client::Client(pin_t ce_pin, pin_t cs_pin, uint8_t node_id)
  : node_id(node_id), radio(ce_pin, cs_pin), network(radio), mesh(radio, network)
{}


void Client::init()
{
    // setup mesh
    mesh.setNodeID(node_id);

    // set emmision strengh
    radio.begin();
    radio.setPALevel(RF24_PA_MAX);
}


bool Client::connect(uint8_t retries)
{
    Serial.println(F("Connecting to the mesh..."));
    if (!mesh.begin())
    {
        if (radio.isChipConnected())
        {
            // try to reconnect
            do
            {
                Serial.println(F("Could not connect to network.\nretrying..."));

                // if connection fails too often, try to restart
                retries--;
                if (retries == 0) {utils::restart(); }

            } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
        } else {
            Serial.println(F("Radio hardware not responding."));
            return false;
        }
    }
    return true;
}


void Client::update()
{
    mesh.update();

    // check for available messages
    while (network.available()) {
        RF24NetworkHeader header;
        payload_t payload;
        network.read(header, &payload, sizeof(payload));
        Serial.print("Received packet #");
        Serial.print(payload.counter);
        Serial.print(" at ");
        Serial.println(payload.ms);
    }
}


bool Client::send(payload_t payload, uint8_t target)
{
    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&payload, 'M', sizeof(payload))) {

      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println("Renewing Address");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          //If address renewal fails, reconfigure the radio and restart the mesh
          //This allows recovery from most if not all radio errors
          mesh.begin();
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK");
    }    
}
