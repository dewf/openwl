#include "timer.h"

#include "globals.h"
#include <stdio.h>
#include <set>

#include "private_defs.h"

VOID CALLBACK timerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
	wl_TimerRef timer = (wl_TimerRef)lpParameter;
	PostMessage(appGlobalWindow, WM_WLTimerMessage, 0, (LPARAM)timer);
}

// maintain increasing IDs so that we uniquely identify timers (better than pointers, which could potentially be reused via 'new')
static unsigned int nextTimerID = 0;
static std::set<unsigned int> activeTimers;

void wl_Timer::cancelTimer()
{
	if (!canceled) {
		DeleteTimerQueueTimer(timerQueue, handle, INVALID_HANDLE_VALUE);
		activeTimers.erase(id); // no further events from this timer will be processed, even if they are queued (especially if they are queued!)
		canceled = true;
	}
}

wl_Timer::~wl_Timer()
{
	cancelTimer();
}

wl_TimerRef wl_Timer::create(unsigned int msTimeout, void* userData)
{
	wl_TimerRef timer = new wl_Timer;
	timer->id = nextTimerID++;
	timer->userData = userData;
	QueryPerformanceCounter(&timer->lastPerfCount);
	timer->timerQueue = NULL; // default timer queue ... not sure why we created one to begin with // CreateTimerQueue();

	// let it be known that this is currently a valid timer, so that callbacks can get through
	//  (used to prevent dead timers from firing even if they've managed to queue a callback)
	activeTimers.insert(timer->id);

	if (!CreateTimerQueueTimer(&timer->handle, timer->timerQueue, timerCallback, timer, msTimeout, msTimeout, WT_EXECUTEDEFAULT)) {
		printf("failed to create TimerQueueTimer\n");
		return nullptr;
	}
	return timer;
}

void wl_Timer::onTimerMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	// verify we're still active
	auto found = activeTimers.find(id);
	if (found == activeTimers.end()) {
		//printf("openwl: attempted to process dead timer callback! nothx\n");
		return;
	}

	wl_EventPrivate eventPrivate(message, wParam, lParam);
	wl_Event event = {};
	event._private = &eventPrivate;
	event.eventType = wl_kEventTypeTimer;
	event.handled = false;

	event.timerEvent.timer = this;
	event.timerEvent.userData = userData;
	event.timerEvent.stopTimer = false;

	LARGE_INTEGER perfCount;
	QueryPerformanceCounter(&perfCount);

	auto sinceLast = (double)(perfCount.QuadPart - lastPerfCount.QuadPart);
	sinceLast /= perfCounterTicksPerSecond.QuadPart;
	event.timerEvent.secondsSinceLast = sinceLast;

	eventCallback(nullptr, &event, nullptr); // both window and userdata are null, because no window is the target. maybe in the future use an Application object w/ associated data?

	lastPerfCount = perfCount;

	// custom event so there's no defwindowproc handling
	if (event.handled && event.timerEvent.stopTimer) {
		cancelTimer();
	}
}
