#include "engine/engine.h"
#include "engine/scene.h"

//systems
#include "systems/systems.h"

int main() {
    Scene scene;
    std::vector<System *> systems;
    systems.push_back(&instancerSystem);
    systems.push_back(&renderingSystem);
    auto engine = Engine();
    engine.run(scene, systems);
}
