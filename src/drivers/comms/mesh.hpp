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
        char data[STRING_SIZE];

        void init(uint8_t id, char *mes)
        {
            strncpy(data, mes, STRING_SIZE);
        }
    };
}
