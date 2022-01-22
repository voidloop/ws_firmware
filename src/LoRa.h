#pragma once

namespace LoRa {
    void begin();

    void syncConfig();

    void waitTask();

    void configMode();

    void normalMode();

    void recvMode();
}
