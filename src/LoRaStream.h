#pragma once

class LoRaStream : public Stream {
public:
    size_t write(uint8_t data) override;

    int available() override;

    int availableForWrite() override;

    int read() override;

    int peek() override;

    void flush() override;
};