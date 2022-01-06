#include <Stream.h>
#include "StreamReader.h"

StreamReader::StreamReader(Stream &stream) :
        stream(stream), status(false), buffer(),
        idx(0), discard(false) {
}

int StreamReader::readLine() {
    if (this->status == LINE_AVAILABLE) {
        return LINE_AVAILABLE;
    }

    while (this->stream.available() > 0) {
        char c = (char) this->stream.read();
        // stop condition
        if (c == '\n' && this->idx > 0 && this->buffer[this->idx - 1] == '\r') {
            if (this->discard) {
                this->discard = false;
                this->status = BUFFER_LIMIT_EXCEEDED;
            } else {
                this->buffer[this->idx - 1] = '\0';
                this->status = LINE_AVAILABLE;
            }
            this->idx = 0;
            return this->status;
        } else if (!this->discard) {
            if (this->idx < BUFFER_LEN) {
                this->buffer[this->idx] = c;
                this->idx++;
            } else {
                this->discard = true;
                this->idx = 1;
            }
        } else {
            // discard mode records only last char
            // (used in stop condition)
            this->buffer[0] = c;
        }
    }

    return LINE_UNAVAILABLE;
}


String StreamReader::getString() {
    this->status = LINE_UNAVAILABLE;
    return {this->buffer};
}
