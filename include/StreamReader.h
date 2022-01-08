#ifndef __COMMAND_PARSER__H__
#define __COMMAND_PARSER__H__

#define LINE_AVAILABLE 1
#define LINE_UNAVAILABLE 0
#define BUFFER_LIMIT_EXCEEDED -1

template<int SIZE>
class StreamReader {
public:
    explicit StreamReader(Stream&);

    int read();

    String getString() {
        status = LINE_UNAVAILABLE;
        return {buffer};
    }

    int getStatus() {
        return status;
    }

private:
    Stream &stream;
    int status;
    char buffer[SIZE];
    int idx;
    bool discard;
};

template<int SIZE>
StreamReader<SIZE>::StreamReader(Stream &s) :
        stream(s), status(LINE_UNAVAILABLE), buffer(),
        idx(0), discard(false) {
}

template<int SIZE>
int StreamReader<SIZE>::read() {
    if (status == LINE_AVAILABLE) {
        return LINE_AVAILABLE;
    }

    while (stream.available() > 0) {
        char c = static_cast<char>(stream.read());
        // stop condition
        if (c == '\n' && idx > 0 && buffer[idx - 1] == '\r') {
            if (discard) {
                discard = false;
                status = BUFFER_LIMIT_EXCEEDED;
            } else {
                buffer[this->idx - 1] = '\0';
                status = LINE_AVAILABLE;
            }
            idx = 0;
            return status;
        } else if (!discard) {
            if (idx < SIZE) {
                buffer[idx] = c;
                idx++;
            } else {
                discard = true;
                buffer[0] = c;
                idx = 1;

            }
        } else {
            // discard mode records only last char
            buffer[0] = c;
        }
    }

    return LINE_UNAVAILABLE;
}

#endif // __COMMAND_PARSER__H__
