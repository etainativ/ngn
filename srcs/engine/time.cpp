#include "engine/time.h"

#include <ctime>
#include <time.h>

#define FIXED_UPDATES_PER_SECOND 60
const double FIXED_UPDATE_INTERVAL = 1.0 / FIXED_UPDATES_PER_SECOND;

struct TimeData {
    clock_t lastClock = clock();
    clock_t deltaTime;
    clock_t timeToFixedUpdate = FIXED_UPDATE_INTERVAL;
    tick_t currentTick = 0;
} __timeData;


tick_t getCurrentTick() {
    return __timeData.currentTick;
}


double getDeltaTime() {
    return __timeData.deltaTime;
}


void resetCurrentTick(tick_t newTick) {
    __timeData.currentTick = newTick;
}


void updateDeltaTime() {
    clock_t current = clock();
    __timeData.deltaTime = (current - __timeData.lastClock) / CLOCKS_PER_SEC;
    __timeData.lastClock = current;
    __timeData.timeToFixedUpdate -= __timeData.deltaTime;
}


bool isFixedUpdate() {
    if (__timeData.timeToFixedUpdate < 0) {
	__timeData.timeToFixedUpdate = FIXED_UPDATE_INTERVAL;
	__timeData.currentTick++;
	return true;
    }
    return false;
}
