#pragma once

#include "../openwl.h"

struct wl_Timer {
private:
    unsigned int id;
    void* userData;
    HANDLE timerQueue;
    HANDLE handle;
    //
    LARGE_INTEGER lastPerfCount; // to calculate time since last firing

    wl_Timer() {}
public:
    ~wl_Timer();
    static wl_TimerRef create(unsigned int msTimeout, void* userData);

    void onTimerMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
