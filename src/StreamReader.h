#ifndef __COMMAND_PARSER__H__
#define __COMMAND_PARSER__H__

#define BUFFER_LEN 32

#define LINE_AVAILABLE 1
#define LINE_UNAVAILABLE 0
#define BUFFER_LIMIT_EXCEEDED -1

class StreamReader {
public:
    explicit StreamReader(Stream& stream);
    int readLine();
    String getString();

private:
    Stream& stream;
    int status;
    char buffer[BUFFER_LEN];
    int idx;
    bool discard;
};

#endif // __COMMAND_PARSER__H__
