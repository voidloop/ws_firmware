#include <Arduino.h>
#include <SoftwareSerial.h>

#define LEVEL_SHIFTER_OE_PIN 7
#define LORA_WIRELESS_MODULE_M0_PIN 8
#define LORA_WIRELESS_MODULE_M1_PIN 9
#define SOFTWARE_SERIAL_TRANSMIT_PIN 10
#define SOFTWARE_SERIAL_RECEIVE_PIN 11
#define LORA_WIRELESS_MODULE_AUX_PIN 12
#define BUFFER_LEN 32

SoftwareSerial softwareSerial(SOFTWARE_SERIAL_RECEIVE_PIN,
                              SOFTWARE_SERIAL_TRANSMIT_PIN);
uint8_t buffer[BUFFER_LEN];

/**
 * @brief Arduino setup() function.
 */
void setup() {
  // Configure Arduino pins
  pinMode(LORA_WIRELESS_MODULE_M0_PIN, OUTPUT);
  pinMode(LORA_WIRELESS_MODULE_M1_PIN, OUTPUT);
  pinMode(LORA_WIRELESS_MODULE_AUX_PIN, INPUT);
  pinMode(SOFTWARE_SERIAL_RECEIVE_PIN, INPUT);
  pinMode(SOFTWARE_SERIAL_TRANSMIT_PIN, OUTPUT);
  pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);

  // Put Lora Wireless Module in configuration mode (mode 3)
  digitalWrite(LORA_WIRELESS_MODULE_M0_PIN, HIGH);
  digitalWrite(LORA_WIRELESS_MODULE_M1_PIN, HIGH);
  digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

  // Configure serial communication
  softwareSerial.begin(9600);
  Serial.begin(9600);

  // Wait Lora WiFi Module initialization
  delay(2000);

  // Configure Lora Wifi Module
  uint8_t configCommand[] = {
      0xC0, /* Command: set register */
      0x00, /* Starting address */
      0x06, /* Length */

      0x01, /* High byte of module address */
      0x01, /* Low byte of module address */

      0x62, /* UART serial port rate: 9600
             * Serial parity bit: 8N1
             * Air data rate: 2.4 Kbps */

      0x00, /* Sub-Packet setting: 200 bytes
             * RSSI ambient noise: disable
             * Transmitting power: 22 dBm */

      0x17, /* Channel: 873.125 MHz */

      0x03, /* RSSI byte: disable
             * Transmission method: transparent
             * LTB: disable
             * WOR cycle: 1500 ms */
  };
  const int size = sizeof(configCommand) / sizeof(configCommand[0]);
  softwareSerial.write(configCommand, size);
}

/**
 * @brief Arduino loop() function.
 */
void loop() {
  int nBytes = softwareSerial.available();
  if (nBytes > 0) {
    if (nBytes > BUFFER_LEN) {
      nBytes = BUFFER_LEN;
    }
    softwareSerial.readBytes(buffer, nBytes);
    for (int i = 0; i < nBytes; ++i) {
      Serial.print("c[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(buffer[i], HEX);
    }
  }
  delay(100);
}