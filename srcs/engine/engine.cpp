#include "engine/engine.h"
#include "engine/system.h"

bool isMainLoopRunning = true;
void run(std::vector<System *>& systems)
{
    entt::registry entities;
    struct SystemsData *systemsData = createSystems(systems);
    initSystems(systemsData, &entities);
    while (isMainLoopRunning)
    {
	updateSystems(systemsData, &entities);
    }

    destroySystems(systemsData, &entities);
}
