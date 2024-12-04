/**
 * @file client.hpp
 * @author Nilusink
 * @brief mesh client
 * @date 2024-12-04
 * 
 * @copyright Copyright Nilusink (c) 2024
 * 
 */
#include "mesh.hpp"



namespace mesh
{
    class Server
    {
        private:
            RF24 *radio;
            RF24Network *network;
            RF24Mesh *mesh;

            // MessageBuffer<16, 1024> send_buffer;
            // bool message_written = false;
            // char last_message[64];
            MessageBuffer receive_buffer;

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