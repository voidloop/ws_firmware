#ifndef __CONFIG__H__
#define __CONFIG__H__

#define LORA_MODULE_AUX_PIN 2
#define LEVEL_SHIFTER_OE_PIN 7
#define LORA_MODULE_M0_PIN 8
#define LORA_MODULE_M1_PIN 9
#define SERIAL_TX_PIN 10
#define SERIAL_RX_PIN 11

#define MAX_COMMAND_LEN 64

const uint8_t configCommand[] = {
        0xC0, /* Command: set register */
        0x00, /* Starting address */
        0x08, /* Length */

        0x10, /* Address high byte */
        0x10, /* Address low byte */

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

#endif // __CONFIG__H__