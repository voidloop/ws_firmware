#pragma once

#include <Arduino.h>

class LoRaModule;

class TransmissionMode {
public:
    explicit TransmissionMode(LoRaModule &module) : module(module) {}

    virtual size_t write(uint8_t) = 0;
    virtual ~TransmissionMode() = default;

protected:
    LoRaModule &module;
};
