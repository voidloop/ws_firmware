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

SoftwareSerial serial(RX_PIN, TX_PIN);
volatile size_t byteWritten = 0;

void auxRisingIsr() { byteWritten = 0; }

size_t write(uint8_t data);

void writeTarget();

size_t writeByte(uint8_t data);

bool writeConfig(const Config &config);

bool readConfig(Config &config);

void waitTask();

void configMode();

void normalMode();

//void printConfig(const Config &config);

class FixedStream : public Stream {
public:
    explicit FixedStream() : Stream() {}

    size_t write(uint8_t data) override {
        // new packet only if there is enough space for data, o.w. wait
        if (byteWritten % PACKET_SIZE == 0) {
            if (byteWritten + TARGET_SIZE + 1 > BUFFER_SIZE) {
                waitTask();
            }
            writeTarget();
            return writeByte(data);
        }
        // module buffer overflow: wait and write target
        if (byteWritten + 1 > BUFFER_SIZE) {
            waitTask();
            writeTarget();
        }
        return writeByte(data);
    }

    int available() override { return serial.available(); }

    int availableForWrite() override { return serial.availableForWrite(); }

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

void LoRa::syncConfig() {
    userOutput.println("Reading LoRa configuration...");
    userOutput.flush();

    configMode();
    serial.begin(9600);

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

void writeTarget() {
    writeByte(TARGET_ADDRH);
    writeByte(TARGET_ADDRL);
    writeByte(TARGET_CH);
}

size_t writeByte(uint8_t data) {
    byteWritten++;
    return serial.write(data);
}

void waitTask() {
    while (digitalRead(AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(100);
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