#pragma once
#include <vector>


struct SystemsData;
struct SystemStats {
    float initDuration;
    float lastUpdateDuration;
    float updateDurationAvrage;
    float lastFixedUpdateDuration;
    float fixedUpdateDurationAvrage;
};

struct System {
    const char* name;
    SystemStats stats;
    void (*init)();
    void (*update)();
    void (*fixedUpdate)();
    void (*destroy)();
};


SystemsData *createSystems(std::vector<System*> systemsNames);
void initSystems(SystemsData *data);
void destroySystems(SystemsData *data);
void fixedUpdateSystems(SystemsData *data);
void updateSystems(SystemsData *data);
void registerSystem(System *system);
