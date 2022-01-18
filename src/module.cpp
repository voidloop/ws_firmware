#include <Arduino.h>
#include <BufferedOutput.h>
#include <SafeStringReader.h>
#include <SoftwareSerial.h>

#include "config.h"

extern BufferedOutput userOutput;
extern SafeStringReader loRaReader;
extern BufferedOutput loRaOutput;

SoftwareSerial loRaSerial(RX_PIN, TX_PIN);

const size_t commandSize = 3;
const size_t configSize = 6;

using Config = uint8_t[configSize];

#define CONFIG_BAUD_RATE 9600
#define NORMAL_BAUD_RATE 9600

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
    while (digitalRead(AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void switchConfigMode() {
    digitalWrite(M0_PIN, HIGH);
    digitalWrite(M1_PIN, HIGH);
    waitLoRaTask();
    loRaSerial.begin(CONFIG_BAUD_RATE);
}

void switchNormalMode() {
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    waitLoRaTask();
    loRaSerial.begin(NORMAL_BAUD_RATE);
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

void discardExtraBytes() {
    while (loRaSerial.available()) {
        uint8_t c = loRaSerial.read();
        userOutput.print("Error: Extra byte received: ");
        userOutput.println(c, HEX);
    }
}

void writeLoRaConfig() {
    userOutput.println("Writing LoRa configuration...");
    userOutput.flush();

    switchConfigMode();
    discardExtraBytes();

    uint8_t buffer[commandSize] = {0xC0, 0x00, configSize};
    Config config;

    loRaSerial.write(buffer, commandSize);
    loRaSerial.write(defaultConfig, configSize);

    loRaSerial.readBytes(buffer, commandSize);
    loRaSerial.readBytes(config, configSize);

    waitLoRaTask();
    switchNormalMode();

    printConfig(config);

    userOutput.println("Done.");
    userOutput.flush();
}

void readLoRaConfig() {
    userOutput.println("Reading LoRa configuration...");
    userOutput.flush();

    switchConfigMode();
    discardExtraBytes();

    const uint8_t command[commandSize] = {0xC1, 0x0, configSize};
    uint8_t response[commandSize];
    Config config;

    bool isOk = true;
    loRaSerial.write(command, commandSize);
    loRaSerial.readBytes(response, commandSize);

    for (size_t i = 0; i < commandSize; ++i) {
        if (command[i] != response[i]) {
            isOk = false;
            break;
        }
    }

    if (isOk) {
        loRaSerial.readBytes(config, configSize);
        for (size_t i = 0; i < configSize; ++i) {
            if (config[i] != defaultConfig[i]) {
                userOutput.println("Warning: LoRa is not configured properly");
                break;
            }
        }
        printConfig(config);
    } else {
        userOutput.println("Error: Cannot read LoRa config");
    }

    waitLoRaTask();
    switchNormalMode();

    userOutput.println("Done.");
    userOutput.flush();
}

void beginLoRa() {
    loRaSerial.begin(NORMAL_BAUD_RATE);
    loRaReader.connect(loRaSerial);
    loRaOutput.connect(loRaSerial, NORMAL_BAUD_RATE);
    waitLoRaTask();
    readLoRaConfig();
}