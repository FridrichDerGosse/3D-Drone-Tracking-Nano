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
#include <SPI.h>

#include "RF24Network.h"
#include "RF24Mesh.h"
#include "RF24.h"

#include "drivers/utils.hpp"
#include "drivers/message_buffer.hpp"


namespace mesh
{
    using namespace utils;

    // using payload_t = byte[STRING_SIZE];
    typedef struct {
        byte data[STRING_SIZE];
    } payload_t;
    const uint16_t payload_size = sizeof(byte) * STRING_SIZE;

    #ifdef COMMS_DEBUGGING
    /**
     * @brief prints a payload in a readable format
     * 
     */
    void print_payload(payload_t payload);
    #endif

    /**
     * @brief converts a payload to string format
     * 
     */
    void payload_to_string(payload_t *payload, char *string);

    /**
     * @brief converts a string to payload format
     * 
     */
    void string_to_payload(const char *string, payload_t *payload);
}
