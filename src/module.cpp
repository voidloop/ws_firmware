#include <Arduino.h>
#include <BufferedOutput.h>
#include <SoftwareSerial.h>

#include "config.h"

extern BufferedOutput userOutput;
extern SoftwareSerial loRaSerial;

const size_t commandSize = 3;
const size_t configSize = 6;

using Config = uint8_t[configSize];


const Config defaultConfig = {
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


void waitLoRaTask() {
    // flush user output before start a potential endless loop
//    userOutput.flush();

    while (digitalRead(LORA_MODULE_AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void switchConfigMode() {
    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);
    waitLoRaTask();
}

void switchNormalMode() {
    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);
    waitLoRaTask();
}

void printConfig(const Config &config) {
    for (size_t i = 0; i < configSize; ++i) {
        if (i > 0) {
            userOutput.print(' ');
        }
        userOutput.print("0x");
        userOutput.print(config[i], HEX);
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

void writeLoRaConfig() {
    userOutput.print("Writing LoRa configuration...");

    switchConfigMode();
    flushExtraBytes();

    uint8_t buffer[commandSize] = {0xC0, 0x00, configSize};
    Config config;

    loRaSerial.write(buffer, commandSize);
    loRaSerial.write(defaultConfig, configSize);

    loRaSerial.readBytes(buffer, commandSize);
    loRaSerial.readBytes(config, configSize);

    waitLoRaTask();
    switchNormalMode();

    userOutput.println(" Done");
    printConfig(config);
}


void readLoRaConfig() {
    userOutput.print("Reading LoRa configuration...");

    switchConfigMode();
    flushExtraBytes();

    uint8_t buffer[commandSize] = {0xC1, 0x0, configSize};
    Config config;

    loRaSerial.write(buffer, commandSize);
    loRaSerial.readBytes(buffer, commandSize);
    loRaSerial.readBytes(config, configSize);

    waitLoRaTask();
    switchNormalMode();

    userOutput.println(" Done");

    for (size_t i = 0; i < configSize; ++i) {
        if (config[i] != defaultConfig[i]) {
            userOutput.println("Warning: LoRa is not configured properly");
            break;
        }
    }

    printConfig(config);
}