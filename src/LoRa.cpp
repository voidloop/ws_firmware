#include <Arduino.h>
#include <SoftwareSerial.h>
#include <util/atomic.h>
#include "config.h"
#include "LoRa.h"

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

using namespace LoRa;

SoftwareSerial softwareSerial(RX_PIN, TX_PIN);
LoRaStream loRaStream;

volatile size_t byteWritten = 0;

void printConfig(const Config &config) {
    for (size_t i = 0; i < CONFIG_SIZE; ++i) {
        if (i > 0) {
            Serial.print(' ');
        }
        Serial.print("0x");
        Serial.print(config[i], HEX);
    }
    Serial.println();
}

bool isBadResponse(const uint8_t buffer[COMMAND_SIZE]) {
    if (buffer[0] == 0xFF && buffer[1] == 0xFF && buffer[2] == 0xFF) {
        return true;
    }
    return false;
}

bool writeConfig(const Config &config, Config &response) {
    uint8_t buffer[COMMAND_SIZE] = {0xC0, 0x00, CONFIG_SIZE};
    softwareSerial.write(buffer, COMMAND_SIZE);
    softwareSerial.write(config, CONFIG_SIZE);

    softwareSerial.readBytes(buffer, COMMAND_SIZE);
    if (isBadResponse(buffer)) {
        return false;
    }

    softwareSerial.readBytes(response, CONFIG_SIZE);

    waitTask();
    return true;
}

bool readConfig(Config &config) {
    uint8_t buffer[COMMAND_SIZE] = {0xC1, 0x0, CONFIG_SIZE};
    softwareSerial.write(buffer, COMMAND_SIZE);

    softwareSerial.readBytes(buffer, COMMAND_SIZE);
    if (isBadResponse(buffer)) {
        return false;
    }

    softwareSerial.readBytes(config, CONFIG_SIZE);

    waitTask();
    return true;
}

void LoRa::begin() {
    Serial.println("LoRa initialization started");

    softwareSerial.begin(NORMAL_BAUD_RATE);

    pinMode(AUX_PIN, INPUT);
    pinMode(M0_PIN, OUTPUT);
    pinMode(M1_PIN, OUTPUT);
    waitTask();

    syncConfig();

    attachInterrupt(digitalPinToInterrupt(AUX_PIN), [] {
        byteWritten = 0;
    }, RISING);

    recvMode();
    Serial.println("LoRa is ready");
}

bool configsAreEqual(const Config &a, const Config &b) {
    for (int i = 0; i < CONFIG_SIZE; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

void LoRa::syncConfig() {
    Serial.println("Reading LoRa configuration...");

    configMode();

    Config buffer;
    if (!readConfig(buffer)) {
        Serial.println("Serial error!");
        return;
    }
    printConfig(buffer);

    if (!configsAreEqual(defaultConfig, buffer)) {
        Serial.println("LoRa is not configured");
        Serial.println("Writing configuration...");

        if (!writeConfig(defaultConfig, buffer)) {
            Serial.println("Serial error!");
            return;
        }

        if (!configsAreEqual(defaultConfig, buffer)) {
            Serial.println("Cannot write configuration!");
            return;
        }
    }
}

void LoRa::waitTask() {
    while (digitalRead(AUX_PIN) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(100);
}

void LoRa::configMode() {
    softwareSerial.begin(CONFIG_BAUD_RATE);
    digitalWrite(M0_PIN, HIGH);
    digitalWrite(M1_PIN, HIGH);
    waitTask();
}

void LoRa::normalMode() {
    softwareSerial.begin(NORMAL_BAUD_RATE);
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    waitTask();
}

void LoRa::recvMode() {
    softwareSerial.begin(NORMAL_BAUD_RATE);
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, HIGH);
    waitTask();
}

//-----------------------------------------------------------------------------
// LoRaStream implementation
//-----------------------------------------------------------------------------

size_t writeTarget() {
    softwareSerial.write(TARGET_ADDRH);
    softwareSerial.write(TARGET_ADDRL);
    softwareSerial.write(TARGET_CH);
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
    softwareSerial.write(data);

    noInterrupts();
    byteWritten += count + 1;
    interrupts();
    return 1;
}

size_t LoRaStream::write(const uint8_t *buffer, size_t size) {
    return Stream::write(buffer, size);
}

void LoRaStream::flush() {
    return softwareSerial.flush();
}

int LoRaStream::peek() {
    return softwareSerial.peek();
}

int LoRaStream::read() {
    return softwareSerial.read();
}

int LoRaStream::availableForWrite() {
    return softwareSerial.availableForWrite();
}

int LoRaStream::available() {
    return softwareSerial.available();
}