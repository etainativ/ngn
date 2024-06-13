#include "engine/entities.h"
#include "engine/engine.h"
#include "engine/system.h"
#include "engine/time.h"


bool isMainLoopRunning = true;
void run(std::vector<System *>& systems)
{
    initEntities();
    struct SystemsData *systemsData = createSystems(systems);
    initSystems(systemsData);
    initTime();
    while (isMainLoopRunning)
    {
	updateDeltaTime();
	updateSystems(systemsData);
	if (isFixedUpdate()) {
	    fixedUpdateSystems(systemsData);
	    updateTick();
	};
    }

    destroySystems(systemsData);
    destroyEntities();
}
