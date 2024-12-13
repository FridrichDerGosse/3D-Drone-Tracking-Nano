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
    extern uint8_t connected_clilents[2][2];
    class Server
    {
        private:
            RF24 *radio;
            RF24Network *network;
            RF24Mesh *mesh;

            MessageBuffer receive_buffer;

        protected:
            void try_set_connected(uint8_t node_id);

        public:
            bool clients_connected();

            Server(RF24 *radio, RF24Network *network, RF24Mesh *mesh);

            /**
             * @brief initial setup of the mesh
             * 
             */
            bool init();

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