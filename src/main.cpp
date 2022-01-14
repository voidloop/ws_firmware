#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SafeStringReader.h>
#include <BufferedOutput.h>
#include <ArduinoJson.h>

#include "config.h"
#include "module.h"

createSafeStringReader(userReader, 32, "\r\n")
createBufferedOutput(userOut, 66, DROP_UNTIL_EMPTY)

createSafeStringReader(moduleReader, 64, "\r\n")
createBufferedOutput(moduleOut, 66, BLOCK_IF_FULL)

SoftwareSerial moduleSerial(LORA_MODULE_RX_PIN, LORA_MODULE_TX_PIN);


void setup() {
    pinMode(LORA_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_MODULE_AUX_PIN, INPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    Serial.begin(9600);
    SafeString::setOutput(Serial); // DEBUG

    userReader.connect(Serial);
    userOut.connect(Serial);

    moduleSerial.begin(9600);
    readLoRaModuleConfig();

    moduleOut.connect(moduleSerial, 9600);
    moduleReader.connect(moduleSerial);
    userOut.flush();
}

void runStatusCommand(BufferedOutput &output) {
    StaticJsonDocument<200> doc;
    doc["hello"] = "world";
    serializeJson(doc, output);
    output.println();
}

inline void errorInvalidCommandSyntax() {
    userOut.println("Error: Invalid command syntax");
}

inline void errorUnknownCommand() {
    userOut.println("Error: Unknown command");
}

void handleUserCommand() {
    cSF(command, 11)
    int idx = 0;

    idx = userReader.stoken(command, idx, ' ');
    command.toLowerCase();

    if (command.equals("check")) {
        if (idx != -1) {
            errorInvalidCommandSyntax();
            return;
        }
        readLoRaModuleConfig();
    } else if (command.equals("init")) {
        if (idx != -1) {
            errorInvalidCommandSyntax();
            return;
        }
        writeLoRaModuleConfig();
    } else if (command.equals("status")) {
        if (idx != -1) {
            errorInvalidCommandSyntax();
            return;
        }
        runStatusCommand(userOut);
    } else {
        errorUnknownCommand();
    }
}

void handleModuleCommand() {
    userOut.println(moduleReader);
    const uint8_t addr[] = {0x01, 0x01, 0x17};
    moduleOut.write(addr, 3);
    runStatusCommand(moduleOut);
}

void processInput(SafeStringReader &reader, void (&handler)()) {
    if (reader.read()) {
        reader.trim();
        handler();
    }
}

void loop() {
    userOut.nextByteOut();
    processInput(userReader, handleUserCommand);
    moduleOut.nextByteOut();
    processInput(moduleReader, handleModuleCommand);
}