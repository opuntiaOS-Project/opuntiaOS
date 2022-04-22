#include "LaunchWatchdog.h"

namespace LaunchServer {

LaunchWatchdog* s_LaunchServer_LaunchWatchdog_the = nullptr;

LaunchWatchdog::LaunchWatchdog()
    : LFoundation::EventReceiver()
{
    s_LaunchServer_LaunchWatchdog_the = this;
}

}