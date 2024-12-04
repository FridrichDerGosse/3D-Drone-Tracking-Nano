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

    using payload_t = char[STRING_SIZE];
    const uint16_t payload_size = sizeof(char) * STRING_SIZE;
}
