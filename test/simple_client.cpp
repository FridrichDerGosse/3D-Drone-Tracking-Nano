#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

RF24 radio(9, 10);  // CE, CSN pins

const byte read_address[5] = {0x00, 0x00, 0x00, 0x00, 0x2};  // Correct 5-byte address
const byte write_address[5] = {0x00, 0x00, 0x00, 0x00, 0x01};  // Correct 5-byte address

void setup() {
    Serial.begin(9600);
    printf_begin();

    Serial.println("starting setup");

    radio.begin();
    radio.openWritingPipe(write_address);  // Open a writing pipe
    radio.openReadingPipe(0, read_address);  // Open a reading pipe
    radio.stopListening();  // Stop listening and prepare to send

    radio.setChannel(100);  // Default channel: 76
    radio.setDataRate(RF24_1MBPS);  // Default data rate: 1Mbps

    radio.setAutoAck(false);

    Serial.println("config done");

    radio.printDetails();
    Serial.println("setup done");
}

void loop() {
    char text[32] = "Hello, Master!";
    bool success = radio.write(&text, 32);  // Send data
    if (success) {
        Serial.println("Message sent successfully");
    } else {
        Serial.println("Failed to send message");
    }

    radio.startListening();

    delay(10);

    // wait 100 ms for response
    int start = millis();
    while (!radio.available() && (millis() - start < 1000)) {}

    if (radio.available())
    {
        radio.read(&text, 32);  // Read received data
        Serial.print("Received: ");
        Serial.println(text);
    } else {
        Serial.println("failed to receive ack");
    }

    radio.stopListening();

    delay(1000);
}


/*
SPI Speedz      = 10 Mhz
STATUS          = 0x0e RX_DR=0 TX_DS=0 MAX_RT=0 RX_P_NO=7 TX_FULL=0
RX_ADDR_P0-1    = 0x3130303030 0xc2c2c2c2c2
RX_ADDR_P2-5    = 0xc3 0xc4 0xc5 0xc6
TX_ADDR         = 0x3130303030
RX_PW_P0-6      = 0x20 0x20 0x20 0x20 0x20 0x20
EN_AA           = 0x3f
EN_RXADDR       = 0x03
RF_CH           = 0x64
RF_SETUP        = 0x07
CONFIG          = 0x0e
DYNPD/FEATURE   = 0x00 0x00
Data Rate       = 1 MBPS
Model           = nRF24L01+
CRC Length      = 16 bits
PA Power        = PA_MAX
ARC             = 15
*/
