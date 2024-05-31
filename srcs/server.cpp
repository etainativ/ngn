#include "engine/engine.h"
#include "systems/systems.h"

int main() {
    std::vector<System *> systems;
    systems.push_back(&serverNetworkSystem);
    systems.push_back(&velocitySystem);
    run(systems);
}
