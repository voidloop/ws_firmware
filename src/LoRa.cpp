#include <Arduino.h>
#include <BufferedOutput.h>
#include <SafeStringReader.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "LoRa.h"

extern BufferedOutput userOutput;
extern SafeStringReader loRaReader;
extern BufferedOutput loRaOutput;

#define CONFIG_SIZE 6
#define TARGET_SIZE 3
#define COMMAND_SIZE 3

#define BUFFER_SIZE 400
#define PACKET_SIZE 200

using Config = uint8_t[CONFIG_SIZE];

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

SoftwareSerial serial(RX_PIN, TX_PIN);
volatile size_t byteWritten = 0;
uint8_t target[TARGET_SIZE] = {0x01, 0x01, 0x17};

void auxRisingIsr() { byteWritten = 0; }

size_t write(uint8_t data);

void writeTarget();

size_t internalWrite(uint8_t data);

bool setConfig(const Config &config);

bool getConfig(Config &config);

void waitTask();

void configMode();

void normalMode();


class FixedStream : public Stream {
public:
    explicit FixedStream() : Stream() {}

    int available() override { return serial.available(); }

    int availableForWrite() override { return serial.availableForWrite(); }

    size_t write(uint8_t data) override {
        // new packet only if there is enough space for data, o.w. wait
        if (byteWritten % PACKET_SIZE == 0) {
            if (BUFFER_SIZE + TARGET_SIZE + 1 > BUFFER_SIZE) {
                waitTask();
            }
            writeTarget();
            return internalWrite(data);
        }

        // module buffer overflow: wait and write target
        if (byteWritten + 1 > BUFFER_SIZE) {
            waitTask();
            writeTarget();
        }

        return internalWrite(data);
    }

    int read() override { return serial.read(); }

    int peek() override { return serial.peek(); }

    void flush() override { return serial.flush(); }
};

void LoRa::begin() {
    userOutput.println("LoRa initialization started");
    userOutput.flush();
    pinMode(AUX_PIN, INPUT);
    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    waitTask();

    syncConfig();

    static FixedStream fixedStream;
    loRaReader.connect(serial);
    loRaOutput.connect(fixedStream, 9600);

    attachInterrupt(digitalPinToInterrupt(AUX_PIN), auxRisingIsr, RISING);

    userOutput.println("LoRa is ready");
    userOutput.flush();
}

void printConfig(const Config &config) {
    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
        if (i > 0) {
            userOutput.print(' ');
        }
        userOutput.print("0x");
        userOutput.print(config[i], HEX);
    }
    userOutput.println();
}

void LoRa::syncConfig() {
    userOutput.println("Reading LoRa configuration...");
    userOutput.flush();
    configMode();
    serial.begin(9600);

    Config config;
    getConfig(config);
    printConfig(config);
    waitTask();

    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
        if (config[i] != defaultConfig[i]) {
            userOutput.println("Warning: the module is misconfigured, reconfiguring...");
            userOutput.flush();
            setConfig(defaultConfig);
            waitTask();
            break;
        }
    }

    normalMode();
}

void writeTarget() {
    for (uint8_t b: target) {
        internalWrite(b);
    }
}

size_t internalWrite(uint8_t data) {
    byteWritten++;
    return serial.write(data);
}

void waitTask() {
    while (digitalRead(AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void configMode() {
    digitalWrite(M0_PIN, HIGH);
    digitalWrite(M1_PIN, HIGH);
    waitTask();
}

void normalMode() {
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    waitTask();
}

bool setConfig(const Config &config) {
    configMode();

    serial.begin(9600);

    uint8_t buffer[COMMAND_SIZE] = {0xC0, 0x00, COMMAND_SIZE};
    Config response;

    serial.write(buffer, COMMAND_SIZE);
    serial.write(config, CONFIG_SIZE);

    serial.readBytes(buffer, COMMAND_SIZE);
    serial.readBytes(response, CONFIG_SIZE);

    waitTask();
    normalMode();

    return true;
}

bool getConfig(Config &config) {
    uint8_t buffer[COMMAND_SIZE] = {0xC1, 0x0, CONFIG_SIZE};
    serial.write(buffer, COMMAND_SIZE);
    serial.readBytes(buffer, COMMAND_SIZE);
    serial.readBytes(config, CONFIG_SIZE);
    return true;
}

