#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
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
            loRaStream.read();
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
    LoRa::waitTask();
}

constexpr uint8_t interruptNum = digitalPinToInterrupt(AUX_PIN);

void wakeUp() {
    sleep_disable();
    detachInterrupt(interruptNum);
    attachInterrupt(interruptNum, LoRa::resetByteWritten, RISING);
}

void loop() {
    Serial.println("Going to sleep...");
    sleep_enable();
    detachInterrupt(interruptNum);
    attachInterrupt(interruptNum, wakeUp, RISING);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    sleep_cpu();

    Serial.println("Woke up!");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    updateValues();
    flushInput();
    LoRa::normalMode();
    sendData();
    LoRa::recvMode();
}