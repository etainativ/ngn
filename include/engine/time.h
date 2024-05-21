#pragma once
#include <stdint.h>

typedef uint32_t tick_t;

tick_t getCurrentTick();
double getDeltaTime();
void updateDeltaTime();
bool isFixedUpdate();
void resetCurrentTick(tick_t newTick);
