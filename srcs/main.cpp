#include "engine/engine.h"

//systems
#include "systems/systems.h"

int main() {
    std::vector<System *> systems;
    systems.push_back(&instancerSystem);
    systems.push_back(&cameraSystem);
    systems.push_back(&renderingSystem);
    run(systems);
}
