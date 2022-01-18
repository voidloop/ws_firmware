#include <Arduino.h>
#include <BufferedOutput.h>
#include <SafeStringReader.h>
#include <SoftwareSerial.h>

#include "config.h"
#include "LoRaModule.h"

extern BufferedOutput userOutput;
extern SafeStringReader loRaReader;
extern BufferedOutput loRaOutput;

LoRaModule loRaModule(RX_PIN, TX_PIN, M0_PIN, M1_PIN, AUX_PIN);

const LoRaModule::Config defaultConfig = {
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


void printConfig(const LoRaModule::Config &config) {
    for (size_t i = 0; i < LoRaModule::configSize; ++i) {
        if (i > 0) {
            userOutput.print(' ');
        }
        userOutput.print("0x");
        userOutput.print(config[i], HEX);
    }
    userOutput.println();
}

void writeLoRaConfig() {
    userOutput.println("Writing LoRa configuration...");
    userOutput.flush();

    loRaModule.setConfig(defaultConfig);
    printConfig(defaultConfig);

    userOutput.println("Done.");
    userOutput.flush();
}

void readLoRaConfig() {
    userOutput.println("Reading LoRa configuration...");
    userOutput.flush();

    LoRaModule::Config config;
    loRaModule.getConfig(config);
    printConfig(config);

    userOutput.println("Done.");
    userOutput.flush();
}


void beginLoRa() {
    loRaModule.begin();
    loRaReader.connect(loRaModule);
    loRaOutput.connect(loRaModule, 9600);
    readLoRaConfig();
    loRaModule.fixedMode();
}
