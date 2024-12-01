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


namespace mesh
{
    using namespace utils;

    struct payload_t {
        unsigned long ms;
        unsigned long counter;
    };

    class Client
    {
        private:
            const uint8_t node_id;
            RF24 *radio;
            RF24Network *network;
            RF24Mesh *mesh;

        public:
            Client(RF24 *radio, RF24Network *network, RF24Mesh *mesh, uint8_t node_id);

            void init();

            /**
             * @brief tries to connect to the master node
             * 
             * @param retries how often to try before restart
             * @return true sucessfully connected
             * @return false hardware error
             */
            bool connect(uint8_t retries = 10);

            void update();
            bool send(payload_t payload, bool renew = true, uint8_t target = 0);
    };
}
