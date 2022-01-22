#include <Arduino.h>
#include <BufferedOutput.h>
#include <SafeStringReader.h>
#include <SoftwareSerial.h>
#include <util/atomic.h>
#include "config.h"
#include "LoRa.h"
#include "LoRaStream.h"

extern BufferedOutput userOutput;
extern SafeStringReader loRaReader;
extern BufferedOutput loRaOutput;

#define CONFIG_BAUD_RATE 9600
#define NORMAL_BAUD_RATE 9600
#define CONFIG_SIZE 6
#define TARGET_SIZE 3
#define COMMAND_SIZE 3
#define BUFFER_SIZE 400
#define PACKET_SIZE 200

using Config = uint8_t[CONFIG_SIZE];

const Config defaultConfig = {
        LOCAL_ADDRH, /* Address high byte */
        LOCAL_ADDRL, /* Address low byte */

        0x62, /* UART: 9600,8,N,1 */

        0x00, /* Sub-Packet setting: 200 bytes (PACKET_SIZE)
               * RSSI ambient noise: disable
               * Transmitting power: 22 dBm */

        LOCAL_CH,   /* Channel: 873.125 MHz */

        0x63, /* RSSI byte: disable
               * Transmission method: fixed
               * LTB: disable
               * WOR cycle: 1500 ms */
};

namespace LoRa {
    SoftwareSerial serial(RX_PIN, TX_PIN);
    volatile size_t byteWritten = 0;
}

using namespace LoRa;

bool writeConfig(const Config &config);

bool readConfig(Config &config);

void waitTask();

void configMode();

void normalMode();

//void printConfig(const Config &config);

void LoRa::begin() {
    userOutput.println("LoRa initialization started");
    userOutput.flush();
    pinMode(AUX_PIN, INPUT);
    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    waitTask();

    syncConfig();

    static LoRaStream stream;
    loRaReader.connect(serial);
    loRaOutput.connect(stream, 9600);

    attachInterrupt(digitalPinToInterrupt(AUX_PIN), [] {
        byteWritten = 0;
    }, RISING);

    userOutput.flush();
}

void LoRa::syncConfig() {
    userOutput.println("Reading LoRa configuration...");
    userOutput.flush();

    configMode();

    Config config;
    readConfig(config);
    waitTask();
    //printConfig(config);

    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
        if (config[i] != defaultConfig[i]) {
            userOutput.println("Warning: the module is misconfigured, reconfiguring...");
            userOutput.flush();
            writeConfig(defaultConfig);
            waitTask();
            break;
        }
    }

    normalMode();
    userOutput.println("LoRa is ready.");
}

//void printConfig(const Config &config) {
//    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
//        if (i > 0) {
//            userOutput.print(' ');
//        }
//        userOutput.print("0x");
//        userOutput.print(config[i], HEX);
//    }
//    userOutput.println();
//}

void waitTask() {
    while (digitalRead(AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(100);
}

void configMode() {
    serial.begin(CONFIG_BAUD_RATE);
    digitalWrite(M0_PIN, HIGH);
    digitalWrite(M1_PIN, HIGH);
    waitTask();
}

void normalMode() {
    serial.begin(NORMAL_BAUD_RATE);
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    waitTask();
}

bool isBadResponse(const uint8_t buffer[COMMAND_SIZE]) {
    if (buffer[0] == 0xFF && buffer[1] == 0xFF && buffer[2] == 0xFF) {
        return true;
    }
    return false;
}

bool writeConfig(const Config &config) {
    uint8_t buffer[COMMAND_SIZE] = {0xC0, 0x00, CONFIG_SIZE};
    serial.write(buffer, COMMAND_SIZE);
    serial.write(config, CONFIG_SIZE);

    serial.readBytes(buffer, COMMAND_SIZE);
    if (isBadResponse(buffer)) {
        return false;
    }

    Config tmp;
    serial.readBytes(tmp, CONFIG_SIZE);
    return true;
}

bool readConfig(Config &config) {
    uint8_t buffer[COMMAND_SIZE] = {0xC1, 0x0, CONFIG_SIZE};
    serial.write(buffer, COMMAND_SIZE);

    serial.readBytes(buffer, COMMAND_SIZE);
    if (isBadResponse(buffer)) {
        return false;
    }

    serial.readBytes(config, CONFIG_SIZE);
    return true;
}

//-----------------------------------------------------------------------------
// LoRaStream implementation
//-----------------------------------------------------------------------------

size_t writeTarget() {
    serial.write(TARGET_ADDRH);
    serial.write(TARGET_ADDRL);
    serial.write(TARGET_CH);
    return TARGET_SIZE;
}

size_t LoRaStream::write(uint8_t data) {
    noInterrupts();
    const size_t written = byteWritten;
    interrupts();

    size_t count = 0;
    if (written % PACKET_SIZE == 0) {
        // wait if there is no enough space in the buffer
        if (written + TARGET_SIZE + 1 >= BUFFER_SIZE) {
            waitTask();
        }
        count = writeTarget();
    } else if (written >= BUFFER_SIZE) {
        waitTask();
        count = writeTarget();
    }
    serial.write(data);

    noInterrupts();
    byteWritten += count + 1;
    interrupts();
    return 1;
}

void LoRaStream::flush() {
    return serial.flush();
}

int LoRaStream::peek() {
    return serial.peek();
}

int LoRaStream::read() {
    return serial.read();
}

int LoRaStream::availableForWrite() {
    return serial.availableForWrite();
}

int LoRaStream::available() {
    return serial.available();
}