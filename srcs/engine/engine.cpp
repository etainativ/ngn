#include "engine/engine.h"
#include "engine/system.h"
#include "engine/time.h"

bool isMainLoopRunning = true;
void run(std::vector<System *>& systems)
{
    entt::registry entities;
    struct SystemsData *systemsData = createSystems(systems);
    initSystems(systemsData, &entities);
    while (isMainLoopRunning)
    {
	updateDeltaTime();
	updateSystems(systemsData, &entities);
	if (isFixedUpdate()) {
	    fixedUpdateSystems(systemsData, &entities);
	};
    }

    destroySystems(systemsData, &entities);
}
