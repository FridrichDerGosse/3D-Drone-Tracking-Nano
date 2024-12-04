/**
 * @file mesh.hpp
 * @author Nilusink
 * @brief nRF24 mesh connection
 * @date 2024-11-30
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#pragma once
#include <Arduino.h>
#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>

#include "drivers/utils.hpp"
#include "drivers/message_buffer.hpp"


namespace mesh
{
    using namespace utils;

    struct payload_t {
        uint8_t id;
        const char *data;
    };

    class Client
    {
        private:
            const uint8_t node_id;
            RF24 *radio;
            RF24Network *network;
            RF24Mesh *mesh;

        public:
            bool debugging = false;

            Client(RF24 *radio, RF24Network *network, RF24Mesh *mesh, uint8_t node_id);

            /**
             * @brief initial setup of the mesh
             * 
             */
            void init();

            /**
             * @brief tries to connect to the master node
             * 
             * @param retries how often to try before restart
             * @return true sucessfully connected
             * @return false hardware error
             */
            bool connect(uint8_t retries = 10);

            /**
             * @brief updates mesh, dhcp and reads incomming messages
             * 
             */
            void update();

            /**
             * @brief send a message
             * 
             * @param payload message to send
             * @param renew renew on check connection fail?
             * @param target target node id
             * @return true send successful
             * @return false send fail
             */
            bool send(payload_t payload, bool renew = true, uint8_t target = 0);
    };

    class Server
    {
        private:
            RF24 *radio;
            RF24Network *network;
            RF24Mesh *mesh;

            // MessageBuffer<16, 1024> send_buffer;
            MessageBuffer<16, 1024> *receive_buffer;

        public:
            bool debugging = false;

            Server(RF24 *radio, RF24Network *network, RF24Mesh *mesh);

            /**
             * @brief initial setup of the mesh
             * 
             */
            void init();

            /**
             * @brief tries to start the master node
             * 
             * @return true sucessfully connected
             * @return false hardware error
             */
            bool start();

            /**
             * @brief updates mesh, dhcp and reads incomming messages
             * 
             */
            void update();

            /**
             * @brief send a message
             * 
             * @param payload message to send
             * @param target target node id
             * @return true send successful
             * @return false send fail
             */
            bool send(payload_t payload, uint8_t target);

            /**
             * @brief check if the server has received anything
             * 
             */
            bool available();

            /**
             * @brief get one of the received messages if available
             * 
             * @param buffer message destination
             */
            bool get_received_message(char *buffer);
    };
}
