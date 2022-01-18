#pragma once

#include <SoftwareSerial.h>
#include "TransmissionMode.h"


class LoRaModule : public Stream {
public:
    static const size_t commandSize = 3;
    static const size_t configSize = 6;

    using Config = uint8_t[configSize];

    explicit LoRaModule(uint8_t rx, uint8_t tx, uint8_t m0, uint8_t m1, uint8_t aux);

    ~LoRaModule();

    int available() override { return serial.available(); }

    int availableForWrite() override { return serial.availableForWrite(); }

    int read() override { return serial.read(); }

    int peek() override { return serial.peek(); }

    void flush() override { return serial.flush(); }

    size_t write(uint8_t data) override { return mode->write(data); }

    void transparentMode();

    void fixedMode();

    bool getConfig(Config &config);

    bool setConfig(const Config& config);

    void begin();

private:

    static void auxRisingIsr() {
        byteWritten = 0;
    }

    void waitTask() const;

    void configMode() const;

    void normalMode() const;

    SoftwareSerial serial;
    uint8_t m0, m1, aux;

    // TODO: add interrupt for reset counters
    volatile size_t byteWritten = 0;
    const size_t bufferSize = 400;
    const size_t packetSize = 32;

    TransmissionMode *mode;

    friend class FixedMode;

    friend class TransmissionMode;

    friend class TransparentMode;
};
