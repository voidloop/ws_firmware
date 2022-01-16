#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SafeStringReader.h>
#include <BufferedOutput.h>
#include <ArduinoJson.h>

#include "config.h"
#include "module.h"

createSafeStringReader(userReader, 32, "\r\n")
createBufferedOutput(userOutput, 66, DROP_UNTIL_EMPTY)

createSafeStringReader(loRaReader, 32, "\r\n")
createBufferedOutput(loRaOutput, 66, BLOCK_IF_FULL)

SoftwareSerial loRaSerial(LORA_MODULE_RX_PIN, LORA_MODULE_TX_PIN);


class SerialWrapper : public Stream {
public:
    explicit SerialWrapper(Stream &s) :
            Stream(), stream(s) {
    }

    size_t write(uint8_t data) override {
        return stream.write(data);

        loRaSerial.end()
    }

    int available() override {
        return stream.available();
    }

    int read() override {
        return stream.read();
    }

    int peek() override {
        return stream.peek();
    }

    void flush() override {
        stream.flush();
    }

private:
    Stream &stream;
};

SerialWrapper wrapper(loRaSerial);


void setup() {
    pinMode(LORA_MODULE_M0_PIN, OUTPUT);
    pinMode(LORA_MODULE_M1_PIN, OUTPUT);
    pinMode(LORA_MODULE_AUX_PIN, INPUT);
    pinMode(LEVEL_SHIFTER_OE_PIN, OUTPUT);
    digitalWrite(LEVEL_SHIFTER_OE_PIN, HIGH);

    Serial.begin(115200);
    loRaSerial.begin(9600);

    SafeString::setOutput(Serial); // DEBUG

    userReader.connect(Serial);
    userOutput.connect(Serial);

    readLoRaConfig();

    loRaReader.connect(loRaSerial);
    loRaOutput.connect(wrapper, 9600);

    const uint8_t target[] = {0x01, 0x01, 0x17};
    loRaSerial.write(target, 3);

}

void statusCommand(BufferedOutput &output) {
    StaticJsonDocument<100> doc;
    doc["hello"] = "world";
    serializeJson(doc, output);
    output.println();
}

void userUnknownCommand() {
    userOutput.println("Error: Unknown command");
}

void loRaUnknownCommand() {
    loRaOutput.println(R"({"error":"Unknown command"})");
}

void handleUserMessage() {
    if (userReader.equalsIgnoreCase("check")) {
        readLoRaConfig();
    } else if (userReader.equalsIgnoreCase("init")) {
        writeLoRaConfig();
    } else if (userReader.equalsIgnoreCase("status")) {
        statusCommand(userOutput);
    } else {
        userUnknownCommand();
    }
}


void handleLoRaMessage() {
    userOutput.print("Message received: '");
    userOutput.print(loRaReader);
    userOutput.println("'");

    void (*fn)();

    if (loRaReader.equalsIgnoreCase("status")) {
        fn = [] { statusCommand(loRaOutput); };
    } else {
        fn = loRaUnknownCommand;
    }

    const size_t targetSize = 3;
    using Target = uint8_t[targetSize];
    const Target target = {0x01, 0x01, 0x17};

    loRaOutput.write(target, targetSize);
    int sent = 3;

    uint8_t buf[4] = {0, 0, '\r', '\n'};

    const int capacity = 200;

    int count = sent;


    for (int i = 0; i < 1000; ++i) {
        buf[0] = (i / 10) % 10 + '0';
        buf[1] = i % 10 + '0';

        userOutput.write(buf, 2);
        userOutput.print(' ');


        if ((sent + 4) > capacity) {
            int avail = capacity - sent;
            loRaOutput.write(buf, avail);
            loRaOutput.flush();

            if (count >= capacity * 2) {
                waitLoRaTask();
                count = 0;
            }

            loRaOutput.write(target, targetSize);
            loRaOutput.write(&buf[avail], 4 - avail);
            sent = 3 + 4 - avail;
        } else {
            loRaOutput.write(buf, 4);
            sent += 4;
        }

        count += 4;

        userOutput.print(sent);
        userOutput.print(' ');
        userOutput.println(i);
    }

    loRaOutput.println("end");
    loRaOutput.flush();
}

void processInput(SafeStringReader &reader, void (&handler)()) {
    if (reader.read()) {
        reader.trim();
        handler();
    }
}

void loop() {
    userOutput.nextByteOut();
    processInput(userReader, handleUserMessage);
    loRaOutput.nextByteOut();
    processInput(loRaReader, handleLoRaMessage);
}