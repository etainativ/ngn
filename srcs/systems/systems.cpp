#include "engine/systems.h"


struct System __systems_head = {
    .name = "systems head"
};

#define RUN_SYSTEMS(arg, data, entities) \
    for (System *system : data->systems) \
	if ((system)->arg != nullptr) (system)->arg(entities);

struct SystemsData {
    std::vector<System*> systems;
};


void initSystems(SystemsData *data, entt::registry *entities) {
    RUN_SYSTEMS(init, data, entities);
};


void destroySystems(SystemsData *data, entt::registry *entities) {
    RUN_SYSTEMS(destroy, data, entities);
    delete data;
};


void updateSystems(SystemsData *data, entt::registry *entities) {
    RUN_SYSTEMS(update, data, entities);
};


void fixedUpdateSystems(SystemsData *data, entt::registry *entities) {
    RUN_SYSTEMS(fixedUpdate, data, entities);
};


SystemsData *createSystems(std::vector<System*> systemsNames) {
    SystemsData *systems = new SystemsData;
    for (System *system : systemsNames) {
	systems->systems.push_back(system);
    }
    return systems;
}
