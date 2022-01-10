#ifndef __COMMAND_PARSER__H__
#define __COMMAND_PARSER__H__

template<size_t SIZE>
class StreamReader {
public:
    explicit StreamReader(Stream &);

    bool available();

    String readline() {
        reset();
        return {buffer};
    }

    // this function must be called before readline
    bool isBufferOverflow() {
        return overflow;
    }

private:
    Stream &stream;
    char buffer[SIZE + 1];
    size_t idx;
    bool overflow;
    bool isLastCr;
    bool avail;

    void reset() {
        overflow = false;
        avail = false;
        idx = 0;
    }
};

template<size_t SIZE>
StreamReader<SIZE>::StreamReader(Stream &s) :
        stream(s), buffer(), idx(0), overflow(false),
        isLastCr(false), avail(false) {
}

template<size_t SIZE>
bool StreamReader<SIZE>::available() {
    while (!avail && stream.available() > 0) {
        char c = static_cast<char>(stream.read());
        if (c == '\n' && isLastCr) {
            buffer[idx - 1] = '\0';
            avail = true;
        } else if (!overflow) {
            if (idx < SIZE + 1) {
                buffer[idx++] = c;
            } else {
                overflow = true;
            }
        }
        isLastCr = c == '\r';
    }
    return avail;
}

#endif // __COMMAND_PARSER__H__
