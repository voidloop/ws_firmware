#include <Arduino.h>
#include <unity.h>

#include "config.h"
#include "StreamReader.h"
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
    TEST_ASSERT_EQUAL(10, SERIAL_TX_PIN);
}

void test_software_serial_receive_pin_number() {
    TEST_ASSERT_EQUAL(11, SERIAL_RX_PIN);
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


void test_StreamReader_class() {
    const int streamReaderSize = 12;
    TestStream commandStream("TEST");
    StreamReader<streamReaderSize> streamReader(commandStream);
    TEST_ASSERT_EQUAL(false, streamReader.available());

    commandStream.print("\r\n");

    TEST_ASSERT_EQUAL(true, streamReader.available());
    TEST_ASSERT_EQUAL(0, commandStream.available());

    String command = streamReader.readline();

    TEST_ASSERT_EQUAL_STRING("TEST", command.c_str());

    commandStream.print("THIS STRING IS TOO LONG!!!!\r\n012345678912\r\n");
    TEST_ASSERT_EQUAL(true, streamReader.available());
    TEST_ASSERT_EQUAL(true, streamReader.isBufferOverflow());
    TEST_ASSERT_EQUAL_STRING("THIS STRING ", streamReader.readline().c_str());
    TEST_ASSERT_EQUAL(true, streamReader.available());
    TEST_ASSERT_EQUAL(false, streamReader.isBufferOverflow());
    TEST_ASSERT_EQUAL(streamReaderSize, streamReader.readline().length());
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
    RUN_TEST(test_StreamReader_class);
}


void loop() {
    UNITY_END(); // stop unit testing
}


//void configure() {
//    uint8_t commandBytes[] = {
//            0xC0, /* Command: set register */
//            0x00, /* Starting address */
//            0x06, /* Length */
//
//            0x00, /* High byte of module address */
//            0x00, /* Low byte of module address */
//
//            0x62, /* UART userStream port rate: 9600
//                   * Serial parity bit: 8N1
//                   * Air data rate: 2.4 Kbps */
//
//            0x00, /* Sub-Packet setting: 200 bytes
//                   * RSSI ambient noise: disable
//                   * Transmitting power: 22 dBm */
//
//            0x17, /* Channel: 873.125 MHz */
//
//            0x03, /* RSSI byte: disable
//                   * Transmission method: transparent
//                   * LTB: disable
//                   * WOR cycle: 1500 ms */
//    };
//    const int commandSize = sizeof commandBytes / sizeof commandBytes[0];
//    softwareSerial.write(commandBytes, commandSize);
//}