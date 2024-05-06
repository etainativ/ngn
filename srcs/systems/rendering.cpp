#include "engine/systems.h"
#include "entt/entity/fwd.hpp"


#include <iostream>


void renderingUpdate(entt::registry *entities) {
    std::cout << "Updating" << "\n";
}

CREATE_SYSTEM(rendering, nullptr, nullptr, renderingUpdate, nullptr);
