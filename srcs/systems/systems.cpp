#include "engine/systems.h"
#include <vector>


struct SystemsData {
    std::vector<System*> systems;
};


struct System __systems {};
#define RUN_SYSTEM(arg, system, entities) \
    if ((system)->arg != nullptr) (system)->arg(entities);


SystemsData* createSystems(Scene& scene, entt::registry *entities) {
    SystemsData *data = new SystemsData;
    for (System *system : data->systems)
	RUN_SYSTEM(create, system, entities);
    return data;
};


void destroySystems(SystemsData *data, entt::registry *entities) {
    for (System *system : data->systems)
	RUN_SYSTEM(destroy, system, entities);
    delete data;
};


void updateSystems(SystemsData *data, entt::registry *entities) {
    for (System *system : data->systems)
	RUN_SYSTEM(update, system, entities);
};


void fixedUpdateSystems(SystemsData *data, entt::registry *entities) {
    for (System *system : data->systems)
	RUN_SYSTEM(fixedUpdate, system, entities);
};

void registerSystem(System *system) {
    System *__systemsHead = &__systems;
    while (__systemsHead->next != nullptr)
	__systemsHead = __systemsHead->next;
    __systemsHead->next = system;
};
