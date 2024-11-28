#include <Arduino.h>
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

RF24 radio(9, 10);      // CE, CSN pins
RF24Network network(radio);
RF24Mesh mesh(radio, network);

void setup() {
    Serial.begin(115200);
    mesh.setNodeID(0);  // Master node ID is always 0
    mesh.begin();
}

void loop() {
    mesh.update();
    mesh.DHCP();  // Handle dynamic addressing
    
    // Check for incoming data
    while (network.available()) {
        RF24NetworkHeader header;
        char message[32];
        network.read(header, &message, sizeof(message));
        Serial.print("Received from node ");
        Serial.print(header.from_node);
        Serial.print(": ");
        Serial.println(message);
    }
}
