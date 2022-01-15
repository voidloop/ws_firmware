#include <Arduino.h>
#include <unity.h>

#include "config.h"

void test_level_shifter_oe_pin_number() {
    TEST_ASSERT_EQUAL(7, LEVEL_SHIFTER_OE_PIN);
}

void test_lora_wireless_module_m0_pin_number() {
    TEST_ASSERT_EQUAL(10, LORA_MODULE_M0_PIN);
}

void test_lora_wireless_module_m1_pin_number() {
    TEST_ASSERT_EQUAL(11, LORA_MODULE_M1_PIN);
}

void test_software_serial_transmit_pin_number() {
    TEST_ASSERT_EQUAL(9, LORA_MODULE_TX_PIN);
}

void test_software_serial_receive_pin_number() {
    TEST_ASSERT_EQUAL(8, LORA_MODULE_RX_PIN);
}

void test_lora_wireless_module_aux_pin_number() {
    TEST_ASSERT_EQUAL(2, LORA_MODULE_AUX_PIN);
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
}


void loop() {
    UNITY_END(); // stop unit testing
}
