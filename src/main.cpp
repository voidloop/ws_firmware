#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "LoRa.h"

/* packet: 0x80 SPEED_H SPEED_L 0x7E */
constexpr byte START_BYTE = 0x80;
constexpr byte STOP_BYTE = 0x7E;

extern LoRaStream loRaStream;
uint16_t windSpeed = 0;

void setup() {
    pinMode(WIND_SPEED_PIN, INPUT);
    Serial.begin(115200);
    LoRa::begin();
    LoRa::normalMode();
}

void flushInput() {
    size_t available = loRaStream.available();
    while (true) {
        if (available > 0) {
            Serial.print(loRaStream.read());
        } else {
            break;
        }
        available = loRaStream.available();
    }
}

void updateValues() {
    windSpeed = analogRead(WIND_SPEED_PIN);
}

void sendData() {
    uint8_t packet[4] = {START_BYTE, 0x00, 0x00, STOP_BYTE};
    packet[1] = windSpeed >> 8;
    packet[2] = windSpeed & 0x00FF;
    size_t n = loRaStream.write(packet, sizeof packet);
    Serial.print("Data sent (");
    Serial.print(n);
    Serial.println(" bytes)");
}

void handleWakeUp() {
    Serial.print("Wake up signal received");
    flushInput();
    updateValues();
    sendData();
}

void loop() {
    sendData();
    delay(1000);
}