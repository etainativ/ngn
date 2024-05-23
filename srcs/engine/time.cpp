#include "engine/time.h"

#include <ctime>
#include <time.h>

const double FIXED_UPDATE_INTERVAL = 1.0 / FIXED_UPDATES_PER_SECOND;

struct TimeData {
    clock_t lastClock;
    clock_t startClock;
    double deltaTime;
    double accumaltedDeltaTime = 0;
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
    __timeData.deltaTime = double(current - __timeData.lastClock) / CLOCKS_PER_SEC;
    __timeData.lastClock = current;
    __timeData.accumaltedDeltaTime += __timeData.deltaTime;
}


bool isFixedUpdate() {
    if (__timeData.accumaltedDeltaTime > FIXED_UPDATE_INTERVAL ) {
	__timeData.accumaltedDeltaTime -= FIXED_UPDATE_INTERVAL;
	//TODO log if accumaltedDeltaTime is large
	return true;
    }
    return false;
}


void updateTick() {
    __timeData.currentTick++;
}

void initTime() {
    clock_t now = clock();
    __timeData.lastClock = now;
    __timeData.startClock = now;

}
