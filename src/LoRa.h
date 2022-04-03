#pragma once

namespace LoRa {
    void begin();

    void syncConfig();

    void waitTask();

    void configMode();

    void normalMode();

    void recvMode();

    void resetByteWritten();
}

class LoRaStream : public Stream {
public:
    size_t write(uint8_t data) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    int available() override;

    int availableForWrite() override;

    int read() override;

    int peek() override;

    void flush() override;
};