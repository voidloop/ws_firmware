#pragma once

#include "TransmissionMode.h"

class FixedMode : public TransmissionMode {
public:
    explicit FixedMode(LoRaModule &module, uint8_t addrh, uint8_t addrl, uint8_t ch);
    size_t write(uint8_t data) override;

private:
    size_t internalWrite(uint8_t data);

    void writeTarget();

    static const size_t targetSize = 3;
    uint8_t target[targetSize];
};