#include <Arduino.h>
#include <SoftwareSerial.h>
#include <config.h>

SoftwareSerial softwareSerial(SOFTWARE_SERIAL_RECEIVE_PIN,
                              SOFTWARE_SERIAL_TRANSMIT_PIN);

uint8_t buffer[BUFFER_LEN];

void setup() {
    // Configure Arduino pins
    pinMode(LORA_WIRELESS_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_WIRELESS_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_WIRELESS_MODULE_AUX_PIN, INPUT);
    pinMode(SOFTWARE_SERIAL_RECEIVE_PIN, INPUT_PULLUP);
    pinMode(SOFTWARE_SERIAL_TRANSMIT_PIN, OUTPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    // Put LoRa Wireless Module in configuration mode (mode 3)
    digitalWrite(LORA_WIRELESS_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_WIRELESS_MODULE_M1_PIN, HIGH);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    // Configure serial ports
    softwareSerial.begin(9600);
    Serial.begin(9600);

    // Wait LoRa Wireless Module initialization
    delay(2000);

    // Configure LoRa Wireless Module
//    uint8_t commandBytes[] = {
//        0xC0, /* Command: set register */
//        0x00, /* Starting address */
//        0x06, /* Length */
//
//        0x01, /* High byte of module address */
//        0x01, /* Low byte of module address */
//
//        0x62, /* UART userStream port rate: 9600
//                    * Serial parity bit: 8N1
//                    * Air data rate: 2.4 Kbps */
//
//        0x00, /* Sub-Packet setting: 200 bytes
//                    * RSSI ambient noise: disable
//                    * Transmitting power: 22 dBm */
//
//        0x17, /* Channel: 873.125 MHz */
//
//        0x03, /* RSSI byte: disable
//                    * Transmission method: transparent
//                    * LTB: disable
//                    * WOR cycle: 1500 ms */
//    };

    uint8_t commandBytes[] = {0xC1, 0x00, 0x08};
    const int commandSize = sizeof commandBytes / sizeof commandBytes[0];
    softwareSerial.write(commandBytes, commandSize);

    // Waiting and print LoRa Wireless Module configuration
    while (softwareSerial.available() == 0) {}

    Serial.println("LoRa Wireless Module detected!");
    Serial.println("Reading configuration...");

    const int expectedSize = commandSize + commandBytes[2];
    softwareSerial.readBytes(buffer, expectedSize);

    Serial.println();
    uint8_t *data = buffer + 3;
    Serial.print(" Address: ");
    Serial.print(data[0], HEX);
    Serial.print(' ');
    Serial.println(data[1], HEX);
    Serial.print(" Channel: ");
    Serial.println(data[4], HEX);
    Serial.println();

    digitalWrite(LORA_WIRELESS_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_WIRELESS_MODULE_M1_PIN, LOW);
    Serial.println("LoRa Wireless Module ready, starting loop...");
}



void loop() {
    int nBytes = softwareSerial.available();
    if (nBytes > 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        if (nBytes > BUFFER_LEN) {
            nBytes = BUFFER_LEN;
        }
        softwareSerial.readBytes(buffer, nBytes);
        for (int i = 0; i < nBytes; ++i) {
            if (i > 0) {
                Serial.print(' ');
            }
            Serial.print((char) buffer[i]);
        }
        Serial.println();
        digitalWrite(LED_BUILTIN, LOW);
    }
    delay(10);
}