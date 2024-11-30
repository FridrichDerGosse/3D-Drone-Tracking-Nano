#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>


#define FAN_PIN A0
#define FAN_PWM_PIN 6


RF24 radio(9, 10);  // CE, CSN pins

const byte read_address[5] = {0x00, 0x00, 0x00, 0x00, 0x01};  // Correct 5-byte address
const byte write_address[5] = {0x00, 0x00, 0x00, 0x00, 0x02};  // Correct 5-byte address

void setup() {
    // fan stuff
    pinMode(FAN_PIN, INPUT);  // wrong pin, wont match up
    pinMode(FAN_PWM_PIN, OUTPUT);

    analogWrite(FAN_PWM_PIN, 127);

    Serial.begin(9600);
    printf_begin();

    radio.begin();
    radio.openWritingPipe(write_address);  // Open a writing pipe
    radio.openReadingPipe(0, read_address);  // Open a reading pipe
    radio.startListening();  // Start listening

    radio.setChannel(100);  // Default channel: 76
    radio.setDataRate(RF24_1MBPS);  // Default data rate: 1Mbps

    radio.setAutoAck(false);

    radio.printDetails();
    Serial.println("setup done");
}

char text[32];
bool has_received;
void loop()
{
    has_received = false;
    while (radio.available()) {
        radio.read(&text, 32);  // Read received data
        Serial.print("Received: ");
        Serial.println(text);

        // need for callback
        has_received = true;
    }

    if (has_received)
    {
        // send back
        radio.stopListening();
        delay(10);  // delay for switching modes

        strncpy(text, "Hello, client!", 15);
        bool success = radio.write(&text, 32);  // Send data

        if (success) {
            Serial.println("Message sent successfully");
        } else {
            Serial.println("Failed to send message");
        }

        radio.startListening();
        delay(10);  // delay for switching modes
    }
}

/*
SPI Speedz      = 10 Mhz
STATUS          = 0x0e RX_DR=0 TX_DS=0 MAX_RT=0 RX_P_NO=7 TX_FULL=0
RX_ADDR_P0-1    = 0x3130303030 0xcccccccc3c
RX_ADDR_P2-5    = 0x33 0xce 0x3e 0xe3
TX_ADDR         = 0x3130303030
RX_PW_P0-6      = 0x20 0x20 0x20 0x20 0x20 0x20
EN_AA           = 0x3f
EN_RXADDR       = 0x03
RF_CH           = 0x64
RF_SETUP        = 0x07
CONFIG          = 0x0f
DYNPD/FEATURE   = 0x00 0x00
Data Rate       = 1 MBPS
Model           = nRF24L01+
CRC Length      = 16 bits
PA Power        = PA_MAX
ARC             = 15
*/
