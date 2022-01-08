#include <Arduino.h>
#include <Arduino_JSON.h>
#include <SoftwareSerial.h>

#include "config.h"
#include "StreamReader.h"

auto softwareSerial = SoftwareSerial(SERIAL_RX_PIN, SERIAL_TX_PIN);
auto streamReader = StreamReader<MAX_COMMAND_LEN>(softwareSerial);
volatile bool auxRisingEdgeDetected = false;
unsigned long lastAuxRisingEdgeDetectedMillis = 0;

void auxRisingIsr() {
    auxRisingEdgeDetected = true;
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
    auxRisingEdgeDetected = false;
    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);

    while (!auxRisingEdgeDetected);
    delay(10);

    softwareSerial.begin(9600);

    const size_t responseSize = 11;
    uint8_t buffer[responseSize] = {0xC1, 0x00, 0x08};

    Serial.println("Reading LoRa Module Configuration...");
    auxRisingEdgeDetected = false;
    softwareSerial.write(buffer, 3);
    softwareSerial.readBytes(buffer, responseSize);

    while (!auxRisingEdgeDetected);
    delay(10);

    for (uint8_t value: buffer) {
        Serial.print(" 0x");
        Serial.print(value, HEX);
    }
    Serial.println();

    Serial.println("Mode Switching: Normal Mode (0)");
    auxRisingEdgeDetected = false;
    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);

    while (!auxRisingEdgeDetected);
    lastAuxRisingEdgeDetectedMillis = millis();
    delay(10);

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

        // wait until LoRa module is busy before send the response
        if (digitalRead(LORA_MODULE_AUX_PIN) == LOW) {
            auxRisingEdgeDetected = false;
            while(!auxRisingEdgeDetected);
        }
        softwareSerial.print(jsonString + "\r\n");
    } else {
        Serial.println("Error: Command Unknown");
    }
}

void loop() {
//    // detect LoRa module auxRisingEdgeDetected
//    if (auxRisingEdgeDetected || (digitalRead(LORA_MODULE_AUX_PIN) == LOW)) {
//        lastAuxRisingEdgeDetectedMillis = millis();
//        auxRisingEdgeDetected = false;
//    }
//
//    if (millis() - lastAuxRisingEdgeDetectedMillis > 5000) {
//        // sleep
//    }

    if (streamReader.available()) {
        if (streamReader.isBufferOverflow()) {
            Serial.println("Error: Buffer Limit Exceeded");
        } else {
            String line = streamReader.readline();
            if (line.length() > 0) {
                processCommand(line);
            }
        }
    }
}