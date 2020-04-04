#pragma once

#include "../openwl.h"

struct wl_Timer {
private:
    unsigned int id;
    void* userData = nullptr;
    HANDLE timerQueue = NULL;
    HANDLE handle = NULL;
    bool canceled = false;
    //
    LARGE_INTEGER lastPerfCount; // to calculate time since last firing

    wl_Timer() {}
    void cancelTimer();
public:
    ~wl_Timer();
    static wl_TimerRef create(unsigned int msTimeout, void* userData);

    void onTimerMessage(UINT message, WPARAM wParam, LPARAM lParam);
};
