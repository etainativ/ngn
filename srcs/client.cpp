#include "engine/engine.h"
#include "systems/systems.h"

int main() {
    std::vector<System *> systems;
    systems.push_back(&instancerSystem);
    systems.push_back(&clientNetworkSystem);
    systems.push_back(&cameraSystem);
    systems.push_back(&renderingSystem);
    systems.push_back(&inputsSystem);
    systems.push_back(&velocitySystem);
    run(systems);
}
