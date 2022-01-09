#include <Arduino.h>
#include <Arduino_JSON.h>
#include <SoftwareSerial.h>

#include "config.h"
#include "StreamReader.h"

auto softwareSerial = SoftwareSerial(SERIAL_RX_PIN, SERIAL_TX_PIN);
auto wirelessStreamReader = StreamReader<MAX_COMMAND_LEN>(softwareSerial);
// NOLINTNEXTLINE(cppcoreguidelines-interfaces-global-init)
auto userStreamReader = StreamReader<MAX_COMMAND_LEN>(Serial);

void waitUntilBusy() {
    while (digitalRead(LORA_MODULE_AUX_PIN) == LOW);
    delay(50);
}

void switchConfigMode() {
    Serial.print("Mode Switching: Configuration mode (3)... ");

    digitalWrite(LORA_MODULE_M0_PIN, HIGH);
    digitalWrite(LORA_MODULE_M1_PIN, HIGH);
    waitUntilBusy();

    Serial.println("Done");
}

void switchNormalMode() {
    Serial.print("Mode switching: Normal mode (0)... ");

    digitalWrite(LORA_MODULE_M0_PIN, LOW);
    digitalWrite(LORA_MODULE_M1_PIN, LOW);
    waitUntilBusy();

    Serial.println("Done");
}

void printResponse(const uint8_t buffer[], const size_t bufferSize) {
    for (size_t i = 0; i < bufferSize; ++i) {
        if (i > 0) {
            Serial.print(' ');
        }
        Serial.print("0x");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();
}

void readConfig() {
    Serial.println("Reading LoRa module configuration...");

    switchConfigMode();

    const size_t commandSize = 3;
    const size_t payloadSize = 6;
    const size_t bufferSize = commandSize + payloadSize;
    uint8_t buffer[bufferSize] = {0xC1, 0x0, payloadSize};

    softwareSerial.write(buffer, commandSize);
    softwareSerial.readBytes(buffer, bufferSize);
    printResponse(buffer, bufferSize);

    waitUntilBusy();

    switchNormalMode();
}

void writeConfig() {
    Serial.println("Writing LoRa module configuration...");

    switchConfigMode();

    uint8_t buffer[] = {
            0xC0, /* Command: set register */
            0x00, /* Starting address */
            0x08, /* Length */

            0x00, /* Address high byte */
            0x00, /* Address low byte */

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

    const size_t bufferSize = sizeof buffer / sizeof buffer[0];

    softwareSerial.write(buffer, bufferSize);
    softwareSerial.readBytes(buffer, bufferSize);
    printResponse(buffer, bufferSize);

    waitUntilBusy();

    switchNormalMode();
}

void setup() {
    pinMode(LORA_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_MODULE_AUX_PIN, INPUT);
    pinMode(SERIAL_RX_PIN, INPUT);
    pinMode(SERIAL_TX_PIN, OUTPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    Serial.begin(9600);
    softwareSerial.begin(9600);

    waitUntilBusy();

    readConfig();

    Serial.println("Ready");
}

void processCommand(String &command) {
    command.toUpperCase();
    Serial.print("Command: ");
    Serial.println(command);

    if (command.equals("STATUS")) {
        JSONVar jsonObj;
        jsonObj["wind_speed"] = 0;
        jsonObj["wind_direction"] = "NNE";
        jsonObj["temperature"] = 0;
        jsonObj["humidity"] = 0;

        String jsonString = JSON.stringify(jsonObj);
        softwareSerial.print(jsonString + "\r\n");
    } else {
        Serial.println("Error: Command unknown");
    }
}

void processWirelessCommand(const String &command) {

}

void processUserCommand(const String &command) {
    if (command.length() == 0) {
        Serial.println("Ready!");
    } else if (command.equals("INIT")) {
        writeConfig();
    } else if (command.equals("CONFIG")) {
        readConfig();
    } else {
        Serial.println("Error: Unknown command '" + command + "'");
    }
}

void processInput(StreamReader<MAX_COMMAND_LEN> &streamReader, void (*commandFunc)(const String &)) {
    if (streamReader.available()) {
        if (streamReader.isBufferOverflow()) {
            Serial.println("Error: Buffer limit exceeded");
        } else {
            String command = streamReader.readline();
            command.toUpperCase();
            commandFunc(command);
        }
    }
}


void loop() {
//    // detect LoRa module auxRisingEdgeDetected
//    if (auxRisingEdgeDetected || (digitalRead(LORA_MODULE_AUX_PIN) == LOW)) {
//        lastActivityDetected = millis();
//        auxRisingEdgeDetected = false;
//    }
//
//    if (millis() - lastActivityDetected > 5000) {
//        // sleep
//    }
    processInput(userStreamReader, processUserCommand);
    processInput(wirelessStreamReader, processWirelessCommand);
}