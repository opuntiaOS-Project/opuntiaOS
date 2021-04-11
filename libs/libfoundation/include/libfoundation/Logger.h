#include <ostream>

namespace LFoundation::Logger {

extern std::ostream debug;

} // namespace LFoundation::Logger

namespace Logger {
using LFoundation::Logger::debug;
} // namespace Logger