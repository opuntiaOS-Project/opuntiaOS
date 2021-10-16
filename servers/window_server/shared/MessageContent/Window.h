#pragma once

enum WindowType : int {
    Standard = 0,
    Homescreen = 1,
};

enum WindowStatusUpdateType : int {
    Changed,
    Minimized,
    Removed,
};