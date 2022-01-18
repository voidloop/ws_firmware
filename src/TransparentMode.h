#pragma once

#include "LoRaModule.h"

class TransparentMode : public TransmissionMode {
public:
    explicit TransparentMode(LoRaModule &module) : TransmissionMode(module) {}

    size_t write(uint8_t data) override { return module.serial.write(data); }
};
