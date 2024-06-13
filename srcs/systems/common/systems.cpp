#include "engine/system.h"


struct System __systems_head = {
    .name = "systems head"
};

#define RUN_SYSTEMS(arg, data) \
    for (System *system : data->systems) \
	if ((system)->arg != nullptr) (system)->arg();

struct SystemsData {
    std::vector<System*> systems;
};


void initSystems(SystemsData *data) {
    RUN_SYSTEMS(init, data);
};


void destroySystems(SystemsData *data) {
    RUN_SYSTEMS(destroy, data);
    delete data;
};


void updateSystems(SystemsData *data) {
    RUN_SYSTEMS(update, data);
};


void fixedUpdateSystems(SystemsData *data) {
    RUN_SYSTEMS(fixedUpdate, data);
};


SystemsData *createSystems(std::vector<System*> systemsNames) {
    SystemsData *systems = new SystemsData;
    for (System *system : systemsNames) {
	systems->systems.push_back(system);
    }
    return systems;
}
