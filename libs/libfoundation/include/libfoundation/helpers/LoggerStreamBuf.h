#include <__std_streambuffer>

namespace LFoundation::Logger {

class StreamBuf : public std::__stdoutbuf<char> {
public:
    StreamBuf(FILE* file)
        : __stdoutbuf<char>(file)
    {
    }

    ~StreamBuf() = default;
};

} // namespace LFoundation::Logger
