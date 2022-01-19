#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SafeStringReader.h>
#include <BufferedOutput.h>
#include <ArduinoJson.h>
#include "LoRa.h"
#include "config.h"

createSafeStringReader(userReader, 32, "\r\n")
createBufferedOutput(userOutput, 66, DROP_UNTIL_EMPTY)
createSafeStringReader(loRaReader, 32, "\r\n")
createBufferedOutput(loRaOutput, 66, BLOCK_IF_FULL)

void setup() {
    pinMode(OE_PIN, OUTPUT);
    digitalWrite(OE_PIN, HIGH);

    Serial.begin(115200);
    SafeString::setOutput(Serial); // DEBUG

    userReader.connect(Serial);
    userOutput.connect(Serial);

    LoRa::begin();
}

void statusCommand(BufferedOutput &output) {
    StaticJsonDocument<100> doc;
    doc["hello"] = "world";
    serializeJson(doc, output);
    output.println();
}

void userUnknownCommand() {
    userOutput.println("Error: Unknown command");
}

void loRaUnknownCommand() {
    loRaOutput.println(R"({"error":"Unknown command"})");
}

void handleUserMessage() {
    if (userReader.equalsIgnoreCase("check")) {
        //readLoRaConfig();
    } else if (userReader.equalsIgnoreCase("init")) {
        //writeLoRaConfig();
    } else if (userReader.equalsIgnoreCase("status")) {
        statusCommand(userOutput);
    } else {
        userUnknownCommand();
    }
}

void handleLoRaMessage() {
    userOutput.print("Message received: '");
    userOutput.print(loRaReader);
    userOutput.println("'");

    if (loRaReader.equalsIgnoreCase("status")) {
        statusCommand(loRaOutput);
    } else {
        loRaUnknownCommand();
    }
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