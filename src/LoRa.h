#pragma once

namespace LoRa {
    void begin();

    void syncConfig();

    void waitTask();

    void configMode();

    void normalMode();

    void recvMode();
}

class LoRaStream : public Stream {
public:
    size_t write(uint8_t data) override;

    int available() override;

    int availableForWrite() override;

    int read() override;

    int peek() override;

    void flush() override;
};