#include <Arduino.h>
#include <unity.h>

#include "config.h"
#include "TestStream.h"

void test_level_shifter_oe_pin_number() {
    TEST_ASSERT_EQUAL(7, LEVEL_SHIFTER_OE_PIN);
}

void test_lora_wireless_module_m0_pin_number() {
    TEST_ASSERT_EQUAL(8, LORA_MODULE_M0_PIN);
}

void test_lora_wireless_module_m1_pin_number() {
    TEST_ASSERT_EQUAL(9, LORA_MODULE_M1_PIN);
}

void test_software_serial_transmit_pin_number() {
    TEST_ASSERT_EQUAL(10, LORA_MODULE_TX_PIN);
}

void test_software_serial_receive_pin_number() {
    TEST_ASSERT_EQUAL(11, LORA_MODULE_RX_PIN);
}

void test_lora_wireless_module_aux_pin_number() {
    TEST_ASSERT_EQUAL(2, LORA_MODULE_AUX_PIN);
}


void test_TestStream_class() {
    TestStream testStream("This is a");
    testStream.print(" test!");

    TEST_ASSERT_EQUAL_STRING("This is a test!", testStream.getString().c_str());

    String expectedString = String("This ");
    String actualString = String();

    for (unsigned int i = 0; i < expectedString.length(); ++i) {
        char c = static_cast<char>(testStream.read());
        actualString += c;
    }
    TEST_ASSERT_EQUAL_STRING(expectedString.c_str(), actualString.c_str());
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
    RUN_TEST(test_TestStream_class);
}


void loop() {
    UNITY_END(); // stop unit testing
}


// EXPERIMENTAL CODE
//            String line = streamReader.readline();
//            LinkedList<String> args;
//
//            int start = 0;
//            while (true) {
//                char c = line.charAt(start);
//                while (c == ' ') {
//                    start++;
//                    c = line.charAt(start);
//                }
//                if (c == '\0') {
//                    break;
//                }
//                int end = line.indexOf(' ', start);
//                if (end == -1) {
//                    args.add(line.substring(start));
//                    break;
//                } else {
//                    args.add(line.substring(start, end));
//                    start = end;
//                }
//            }
//
//            if (args.size() > 0) {
//                args[0].toUpperCase();
//            }

