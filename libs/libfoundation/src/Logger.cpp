#include <libfoundation/helpers/LoggerStreamBuf.h>
#include <ostream>

namespace LFoundation {

namespace Logger {
    _ALIGNAS_TYPE(std::ostream)
    char debug[sizeof(std::ostream)];
    _ALIGNAS_TYPE(std::ostream)
    char info[sizeof(std::ostream)];
    _ALIGNAS_TYPE(std::ostream)
    char error[sizeof(std::ostream)];
};

class LoggerInit final {
public:
    LoggerInit()
    {
        m_ldebug_ptr = new (Logger::debug) std::ostream(new Logger::StreamBuf(stdout));
    };

    ~LoggerInit()
    {
    }

private:
    std::ostream* m_ldebug_ptr;
};

static LoggerInit logger_init;

}