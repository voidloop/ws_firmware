#include <Arduino.h>
#include <BufferedOutput.h>
#include <SoftwareSerial.h>

#include "config.h"

extern BufferedOutput userOut;
extern SoftwareSerial moduleSerial;

const uint8_t configCommand[] = {
        0xC0, /* Command: set register */
        0x00, /* Starting address */
        0x08, /* Length */

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
        0x00, /* Key high byte */
        0x00  /* Key low byte */
};


void waitLoRaModuleTask() {
    // Flush user output before start a potential endless loop
    userOut.flush();

    while (digitalRead(LORA_MODULE_AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void switchConfigMode() {
    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);
    waitLoRaModuleTask();
}

void switchNormalMode() {
    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);
    waitLoRaModuleTask();
}

void printModuleResponse(const uint8_t buffer[], const size_t bufferSize) {
    for (size_t i = 0; i < bufferSize; ++i) {
        if (i > 0) {
            userOut.print(' ');
        }
        userOut.print("0x");
        userOut.print(buffer[i], HEX);
    }
    userOut.println();
}

void flushExtraBytes() {
    while (moduleSerial.available()) {
        uint8_t c = moduleSerial.read();
        userOut.print("Error: Extra byte received: "); Serial.println(c, HEX);
    }
}

void writeLoRaModuleConfig() {
    userOut.println("Writing LoRa module configuration");
    userOut.flush();
    switchConfigMode();

    flushExtraBytes();

    const size_t bufferSize = sizeof configCommand / sizeof configCommand[0];
    uint8_t buffer[bufferSize];

    moduleSerial.write(configCommand, bufferSize);
    moduleSerial.readBytes(buffer, bufferSize);
    waitLoRaModuleTask();

    printModuleResponse(buffer, bufferSize - 2);

    switchNormalMode();
    userOut.println("Done.");
}



void readLoRaModuleConfig() {
    userOut.println("Reading LoRa module configuration");
    switchConfigMode();
    flushExtraBytes();

    const size_t commandSize = 3;
    const size_t payloadSize = 6;
    const size_t bufferSize = commandSize + payloadSize;
    uint8_t buffer[bufferSize] = {0xC1, 0x0, payloadSize};

    moduleSerial.write(buffer, commandSize);
    moduleSerial.readBytes(buffer, bufferSize);

    const uint8_t *payload = &buffer[3];
    const uint8_t *expected = &configCommand[3];

    for (size_t i = 0; i < min(payloadSize, bufferSize); ++i) {
        if (payload[i] != expected[i]) {
            userOut.println("Warning: The LoRa module is not properly configured");
            break;
        }
    }

    printModuleResponse(buffer, bufferSize);

    waitLoRaModuleTask();
    switchNormalMode();

    userOut.println("Done.");
}