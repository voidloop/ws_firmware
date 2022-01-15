#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SafeStringReader.h>
#include <BufferedOutput.h>
#include <ArduinoJson.h>

#include "config.h"
#include "module.h"

createSafeStringReader(userReader, 32, "\r\n")
createBufferedOutput(userOutput, 66, DROP_UNTIL_EMPTY)

createSafeStringReader(loRaReader, 32, "\r\n")
createBufferedOutput(loRaOutput, 66, BLOCK_IF_FULL)

SoftwareSerial loRaSerial(LORA_MODULE_RX_PIN, LORA_MODULE_TX_PIN);

void setup() {
    pinMode(LORA_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_MODULE_AUX_PIN, INPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    Serial.begin(9600);
    loRaSerial.begin(9600);

    SafeString::setOutput(Serial); // DEBUG

    userReader.connect(Serial);
    userOutput.connect(Serial);

    readModuleConfig();

    loRaReader.connect(loRaSerial);
    loRaOutput.connect(loRaSerial, 9600);

    userOutput.flush();
}

void statusCommand(BufferedOutput &output) {
    StaticJsonDocument<100> doc;
    doc["hello"] = "world";
    serializeJson(doc, output);
    output.println();
}

inline void userErrorUnknownCommand() {
    userOutput.println("Error: Unknown command");
}

inline void loRaErrorUnknownCommand() {
    loRaOutput.println(R"({"error":"Unknown command"})");
}

void handleUserMessage() {
    if (userReader.equalsIgnoreCase("check")) {
        readModuleConfig();
    } else if (userReader.equalsIgnoreCase("init")) {
        writeModuleConfig();
    } else if (userReader.equalsIgnoreCase("status")) {
        statusCommand(userOutput);
    } else {
        userErrorUnknownCommand();
    }
}

void handleLoRaMessage() {
    userOutput.print("Wireless command received: '");
    userOutput.print(loRaReader);
    userOutput.println("'");

    const uint8_t target[] = {0x01, 0x01, 0x17};
    if (loRaReader.equalsIgnoreCase("status")) {
        loRaOutput.write(target, 3);
        statusCommand(loRaOutput);
    } else {
        loRaOutput.write(target, 3);
        loRaErrorUnknownCommand();
    }

    statusCommand(loRaOutput);
}

void processInput(SafeStringReader &reader, void (&handler)()) {
    if (reader.read()) {
        reader.trim();
        handler();
    }
}

void loop() {
    userOutput.nextByteOut();
    processInput(userReader, handleUserMessage);
    loRaOutput.nextByteOut();
    processInput(loRaReader, handleLoRaMessage);
}