#include <Arduino.h>
#include <unity.h>
#include <SoftwareSerial.h>

#include <config.h>

void test_level_shifter_oe_pin_number() {
    TEST_ASSERT_EQUAL(7, LEVEL_SHIFTER_OE_PIN);
}

void test_lora_wireless_module_m0_pin_number() {
    TEST_ASSERT_EQUAL(8, LORA_WIRELESS_MODULE_M0_PIN);
}

void test_lora_wireless_module_m1_pin_number() {
    TEST_ASSERT_EQUAL(9, LORA_WIRELESS_MODULE_M1_PIN);
}

void test_software_serial_transmit_pin_number() {
    TEST_ASSERT_EQUAL(10, SOFTWARE_SERIAL_TRANSMIT_PIN);
}

void test_software_serial_receive_pin_number() {
    TEST_ASSERT_EQUAL(11, SOFTWARE_SERIAL_RECEIVE_PIN);
}

void test_lora_wireless_module_aux_pin_number() {
    TEST_ASSERT_EQUAL(12, LORA_WIRELESS_MODULE_AUX_PIN);
}

void test_buffer_length() {
    TEST_ASSERT_EQUAL(32, BUFFER_LEN);
}

SoftwareSerial softwareSerial(SOFTWARE_SERIAL_RECEIVE_PIN,
                              SOFTWARE_SERIAL_TRANSMIT_PIN);
uint8_t buffer[BUFFER_LEN];


void test_lora_wireless_module_communication() {
    uint8_t command[] = {0xC1, 0x00, 0x08};

    softwareSerial.write(
        command,
        sizeof command / sizeof command[0]);

    delay(1000);

    while (true) {
        int nBytes = softwareSerial.available();
        TEST_ASSERT_GREATER_OR_EQUAL(1, nBytes);

        if (nBytes > BUFFER_LEN) {
            nBytes = BUFFER_LEN;
        } else {
            break;
        }
        softwareSerial.readBytes(buffer, nBytes);
    }
}


void setup() {
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN(); // IMPORTANT LINE!
    RUN_TEST(test_level_shifter_oe_pin_number);
    RUN_TEST(test_lora_wireless_module_m0_pin_number);
    RUN_TEST(test_lora_wireless_module_m1_pin_number);
    RUN_TEST(test_software_serial_transmit_pin_number);
    RUN_TEST(test_software_serial_receive_pin_number);
    RUN_TEST(test_lora_wireless_module_aux_pin_number);
    RUN_TEST(test_buffer_length);

    pinMode(SOFTWARE_SERIAL_TRANSMIT_PIN, OUTPUT);
    pinMode(SOFTWARE_SERIAL_RECEIVE_PIN, INPUT_PULLUP);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);

    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);
    softwareSerial.begin(9600);

    RUN_TEST(test_lora_wireless_module_communication);
}


void loop() {
    UNITY_END(); // stop unit testing
}