#include <Arduino.h>
#include <BufferedOutput.h>
#include <SoftwareSerial.h>

#include "config.h"

extern BufferedOutput userOutput;
extern SoftwareSerial loRaSerial;

const uint8_t defaultConfig[] = {
        0x10, /* Address high byte */
        0x10, /* Address low byte */

        0x62, /* UART: 9600,8,N,1 */

        0x00, /* Sub-Packet setting: 200 bytes
               * RSSI ambient noise: disable
               * Transmitting power: 22 dBm */

        0x17, /* Channel: 873.125 MHz */

        0x63, /* RSSI byte: disable
               * Transmission method: fixed
               * LTB: disable
               * WOR cycle: 1500 ms */
};

const size_t commandSize = (sizeof defaultConfig[0]) * 3;
const size_t payloadSize = (sizeof defaultConfig / sizeof defaultConfig[0]);

void waitModuleTask() {
    // Flush user output before start a potential endless loop
    userOutput.flush();

    while (digitalRead(LORA_MODULE_AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void switchConfigMode() {
    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);
    waitModuleTask();
}

void switchNormalMode() {
    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);
    waitModuleTask();
}

void printModuleConfig(const uint8_t buffer[], const size_t bufferSize) {
    for (size_t i = 0; i < bufferSize; ++i) {
        if (i > 0) {
            userOutput.print(' ');
        }
        userOutput.print("0x");
        userOutput.print(buffer[i], HEX);
    }
    userOutput.println();
}

void flushExtraBytes() {
    while (loRaSerial.available()) {
        uint8_t c = loRaSerial.read();
        userOutput.print("Error: Extra byte received: ");
        userOutput.println(c, HEX);
    }
}

void writeModuleConfig() {
    userOutput.print("Writing LoRa configuration...");
    userOutput.flush();

    switchConfigMode();
    flushExtraBytes();

    const size_t bufferSize = commandSize + payloadSize;
    uint8_t buffer[bufferSize] = {0xC0, 0x00, payloadSize};

    loRaSerial.write(buffer, commandSize);
    loRaSerial.write(defaultConfig, payloadSize);
    loRaSerial.readBytes(buffer, bufferSize);

    waitModuleTask();
    switchNormalMode();

    userOutput.println(" Done");
    printModuleConfig(&buffer[commandSize], payloadSize);
}


void readModuleConfig() {
    userOutput.print("Reading LoRa configuration...");

    switchConfigMode();
    flushExtraBytes();

    const size_t bufferSize = commandSize + payloadSize;
    uint8_t buffer[bufferSize] = {0xC1, 0x0, payloadSize};

    loRaSerial.write(buffer, commandSize);
    loRaSerial.readBytes(buffer, bufferSize);

    waitModuleTask();
    switchNormalMode();

    userOutput.println(" Done");

    const uint8_t *payload = &buffer[commandSize];

    for (size_t i = 0; i < payloadSize; ++i) {
        if (payload[i] != defaultConfig[i]) {
            userOutput.println("Warning: LoRa is not configured properly");
            break;
        }
    }

    printModuleConfig(payload, payloadSize);
}