/**
 * @file client.hpp
 * @author Nilusink
 * @brief mesh client
 * @date 2024-12-04
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#pragma once
#include "mesh.hpp"


namespace mesh
{
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
}