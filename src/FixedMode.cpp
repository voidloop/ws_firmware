#include "LoRaModule.h"
#include "FixedMode.h"

FixedMode::FixedMode(LoRaModule &module, uint8_t addrh, uint8_t addrl, uint8_t ch) :
        TransmissionMode(module), target{addrh, addrl, ch} {
}


size_t FixedMode::write(uint8_t data) {
    // new packet only if there is enough space for data, o.w. wait
    if (module.byteWritten % module.packetSize == 0) {
        if (module.bufferSize + targetSize + 1 > module.bufferSize) {
            module.waitTask();
        }
        writeTarget();
        return internalWrite(data);
    }

    // module buffer overflow: wait and write target
    if (module.byteWritten + 1 > module.bufferSize) {
        module.waitTask();
        writeTarget();
    }

    return internalWrite(data);
}


size_t FixedMode::internalWrite(uint8_t data) {
    module.byteWritten++;
    return module.serial.write(data);
}


void FixedMode::writeTarget() {
    for (uint8_t b: target) {
        internalWrite(b);
    }
}

