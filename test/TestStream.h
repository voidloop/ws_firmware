#pragma once

class TestStream : public Stream {
public:
    explicit TestStream(const char *p) : string(String(p)) {}

    int read() override {
        if (string.length() == 0) {
            return -1;
        } else {
            char c = string[0];
            if (c == '\0') {
                return 0;
            }
            string.remove(0, 1);
            return c;
        }
    }

    int available() override {
        return static_cast<int>(string.length());
    }

    int peek() override {
        if (string.length() == 0) {
            return -1;
        }
        return string[0];
    }

    size_t write(uint8_t c) override {
        if (c == '\0')
            return 0;
        string += static_cast<char>(c);
        return 1;
    }

    String& getString() {
        return string;
    }

private:
    String string;
};
