#pragma once
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"


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
    void (*init)(entt::registry*);
    void (*update)(entt::registry*);
    void (*fixedUpdate)(entt::registry*);
    void (*destroy)(entt::registry*);
};


SystemsData *createSystems(std::vector<System*> systemsNames);
void initSystems(SystemsData *data, entt::registry *entities);
void destroySystems(SystemsData *data, entt::registry *entities);
void fixedUpdateSystems(SystemsData *data, entt::registry *entities);
void updateSystems(SystemsData *data, entt::registry *entities);
void registerSystem(System *system);
