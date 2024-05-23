#pragma once
#include <stdint.h>

#define FIXED_UPDATES_PER_SECOND 60
typedef uint32_t tick_t;
extern const double FIXED_UPDATE_INTERVAL;

tick_t getCurrentTick();
double getDeltaTime();
void updateDeltaTime();
bool isFixedUpdate();
void resetCurrentTick(tick_t newTick);
void updateTick();
void initTime();
