#pragma once
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "scene.h"

#define CREATE_SYSTEM(sname, fcreate, fupdate, ffixedupdate, fdestroy) \
static System system_##sname = { \
    .name = #sname, \
    .stats = {}, \
    .create = fcreate, \
    .update = fupdate, \
    .fixedUpdate = ffixedupdate, \
    .destroy = fdestroy, \
    .next = NULL \
}; \
void  __attribute__ ((constructor)) system_##sname_construct() { \
    registerSystem(&system_##sname);\
};

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
    void (*create)(entt::registry*);
    void (*update)(entt::registry*);
    void (*fixedUpdate)(entt::registry*);
    void (*destroy)(entt::registry*);
    System *next;
};


SystemsData* createSystems(Scene& scene, entt::registry *entities);
void destroySystems(SystemsData *data, entt::registry *entities);
void fixedUpdateSystems(SystemsData *data, entt::registry *entities);
void updateSystems(SystemsData *data, entt::registry *entities);
void registerSystem(System *system);
