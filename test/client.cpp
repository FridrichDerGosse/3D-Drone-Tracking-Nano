/**
 * @file client.cpp
 * @author chat-GPT
 * @brief test for using NRF-24 as MESH
 * @version 0.1
 * @date 2024-11-28
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

RF24 radio(9, 10);      // CE, CSN pins
RF24Network network(radio);
RF24Mesh mesh(radio, network);


void setup() {
    Serial.begin(115200);
    mesh.setNodeID(1);  // Set a unique ID for this node
    mesh.begin();
}


void loop() {
    mesh.update();
    mesh.DHCP();  // Handle dynamic addressing
    
    // Send data to the master node
    if (millis() % 5000 < 50) {  // Send every 5 seconds
        RF24NetworkHeader header(0);  // Send to master node (ID 0)
        char message[] = "Hello, Master!";
        if (network.write(header, &message, sizeof(message))) {
            Serial.println("Message sent successfully");
        } else {
            Serial.println("Failed to send message");
        }
    }
}
