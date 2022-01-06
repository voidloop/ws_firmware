#include <Arduino.h>
#include <SoftwareSerial.h>

#include "config.h"
#include "StreamReader.h"

SoftwareSerial softwareSerial(SOFTWARE_SERIAL_RECEIVE_PIN,
                              SOFTWARE_SERIAL_TRANSMIT_PIN);
StreamReader streamReader(softwareSerial);

void setup() {
    Serial.begin(9600);

    // Configure pins
    pinMode(LORA_WIRELESS_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_WIRELESS_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_WIRELESS_MODULE_AUX_PIN, INPUT);
    pinMode(SOFTWARE_SERIAL_RECEIVE_PIN, INPUT);
    pinMode(SOFTWARE_SERIAL_TRANSMIT_PIN, OUTPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    // Put LoRa Wireless Module in configuration mode (mode 3)
    digitalWrite(LORA_WIRELESS_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_WIRELESS_MODULE_M1_PIN, HIGH);

    // Configure serial ports
    softwareSerial.begin(9600);

    // Wait LoRa Wireless Module
    delay(500);

    const size_t bufferSize = 11;
    uint8_t buffer[bufferSize];
    buffer[0] = 0xC1;
    buffer[1] = 0x00;
    buffer[2] = 0x08;

    Serial.println("Reading LoRa Wireless Module configuration...");
    softwareSerial.write(buffer, 3);
    softwareSerial.readBytes(buffer, bufferSize);

    for (size_t i = 0; i < bufferSize; ++i) {
        if (i > 0) {
            Serial.print(' ');
        }
        Serial.print("0x");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();

    // Put LoRa Wireless Module in transparent mode (mode 0)
    digitalWrite(LORA_WIRELESS_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_WIRELESS_MODULE_M1_PIN, LOW);

    Serial.println("Setup readLine.");

    // Wait LoRa Wireless Module
    delay(500);
}


void processCommand(String& command) {
    command.toUpperCase();
    Serial.print("Command: ");
    Serial.println(command);
    softwareSerial.println("ciccio");
}


void loop() {
    // TODO check AUX pin for every communication process
    if (digitalRead(LORA_WIRELESS_MODULE_AUX_PIN) == HIGH) {
        int rval = streamReader.readLine();
        if (rval == LINE_AVAILABLE) {
            String line = streamReader.getString();
            if (line.length() > 0) {
                processCommand(line);
            }
        } else if (rval == BUFFER_LIMIT_EXCEEDED) {
            Serial.println("Error: buffer limit exceeded");
        }
    }

}