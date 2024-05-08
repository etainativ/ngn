#include "systems/rendering.h"
#include "entt/entity/fwd.hpp"


void renderingCreate(entt::registry *entities) {
}


void renderingUpdate(entt::registry *entities) {
}

struct System renderingSystem = {
    .name = "rendering",
    .stats = {},
    .create = nullptr,
    .update = renderingUpdate,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
