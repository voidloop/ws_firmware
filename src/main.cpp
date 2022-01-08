#include <Arduino.h>
#include <Arduino_JSON.h>
#include <SoftwareSerial.h>

#include "config.h"
#include "StreamReader.h"

SoftwareSerial softwareSerial(SERIAL_RX_PIN, SERIAL_TX_PIN);
// add another byte for the string terminator
StreamReader<MAX_COMMAND_LEN + 1> streamReader(softwareSerial);
volatile bool isLoRaModuleBusy = false;

void auxRisingIsr() {
    isLoRaModuleBusy = false;
}

void setup() {
    Serial.begin(9600);

    pinMode(LORA_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_MODULE_AUX_PIN, INPUT);
    pinMode(SERIAL_RX_PIN, INPUT);
    pinMode(SERIAL_TX_PIN, OUTPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    attachInterrupt(digitalPinToInterrupt(LORA_MODULE_AUX_PIN), auxRisingIsr, RISING);

    Serial.println("Mode Switching: Configuration (3)");
    isLoRaModuleBusy = true;
    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);
    while (isLoRaModuleBusy);
    delay(10);

    softwareSerial.begin(9600);

    const size_t responseSize = 11;
    uint8_t buffer[responseSize] = {0xC1, 0x00, 0x08};

    Serial.println("Reading LoRa Module Configuration...");
    isLoRaModuleBusy = true;
    softwareSerial.write(buffer, 3);
    softwareSerial.readBytes(buffer, responseSize);
    while (isLoRaModuleBusy);

    for (uint8_t value : buffer) {
        Serial.print(" 0x");
        Serial.print(value, HEX);
    }
    Serial.println();

    Serial.println("Mode Switching: Normal Mode (0)");
    isLoRaModuleBusy = true;
    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);
    while (isLoRaModuleBusy);

    detachInterrupt(digitalPinToInterrupt(LORA_MODULE_AUX_PIN));
    Serial.println("Ready.");
}

void processCommand(String &command) {
    command.toUpperCase();
    Serial.print("Command: ");
    Serial.println(command);

    if (command.equals("STATUS")) {
        JSONVar jsonObj;
        jsonObj["wind_speed"] = 0;
        jsonObj["wind_direction"] = "NNE";
        jsonObj["temperature"] = 0;
        jsonObj["humidity"] = 0;

        String jsonString = JSON.stringify(jsonObj);

        // wait until LoRa module is busy
        while (digitalRead(LORA_MODULE_AUX_PIN) == LOW);
        softwareSerial.print(jsonString + "\r\n");
    } else {
        Serial.println("Error: Command Unknown");
    }
}

void loop() {
    if (streamReader.read() != LINE_UNAVAILABLE) {
        if (streamReader.getStatus() == BUFFER_LIMIT_EXCEEDED) {
            Serial.println("Error: Buffer Limit Exceeded");
        } else {
            String line = streamReader.getString();
            if (line.length() > 0) {
                processCommand(line);
            }
        }
    }
}