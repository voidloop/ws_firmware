#include "LoRaModule.h"
#include "TransmissionMode.h"
#include "TransparentMode.h"
#include "FixedMode.h"

void auxRisingIsr() {

}


LoRaModule::LoRaModule(uint8_t rx, uint8_t tx, uint8_t m0, uint8_t m1, uint8_t aux) :
        serial(SoftwareSerial(rx, tx)), m0(m0), m1(m1), aux(aux), mode(new TransparentMode(*this)) {
}

LoRaModule::~LoRaModule() {
    delete mode;
}

void LoRaModule::waitTask() const {
    while (digitalRead(aux) == LOW);
    // Datasheet: the general recommendation is to detect the output state of the AUX
    // pin and switch after 2ms when the output is high.
    // Increase this delay if the LoRa module doesn't save or read the config correctly.
    delay(50);
}

void LoRaModule::configMode() const {
    digitalWrite(m0, HIGH);
    digitalWrite(m1, HIGH);
    waitTask();
}

void LoRaModule::normalMode() const {
    digitalWrite(m0, LOW);
    digitalWrite(m1, LOW);
    waitTask();
}

void LoRaModule::transparentMode() {
    delete mode;
    mode = new TransparentMode(*this);
}

void LoRaModule::fixedMode() {
    delete mode;
    mode = new FixedMode(*this, 0x01, 0x01, 0x17);
}

// TODO: check errors
bool LoRaModule::setConfig(const Config &config) {
    configMode();

    serial.begin(9600);

    uint8_t buffer[commandSize] = {0xC0, 0x00, configSize};
    Config response;

    serial.write(buffer, commandSize);
    serial.write(config, configSize);

    serial.readBytes(buffer, commandSize);
    serial.readBytes(response, configSize);

    waitTask();
    normalMode();

    return true;
}

// TODO: check errors
bool LoRaModule::getConfig(Config &config) {
    configMode();

    //serial.begin(9600);

    uint8_t buffer[commandSize] = {0xC1, 0x0, configSize};
    serial.write(buffer, commandSize);
    serial.readBytes(buffer, commandSize);
    serial.readBytes(config, configSize);
    waitTask();
    normalMode();
    return true;
}



void LoRaModule::begin() {
    pinMode(aux, INPUT);
    pinMode(m0, OUTPUT);
    pinMode(m1, OUTPUT);
    serial.begin(9600);
    // wait module init
    waitTask();
}
