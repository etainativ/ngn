#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

entt::registry *entities;

void initEntities() {
    entities = new entt::registry;
}

void destroyEntities() {
    delete entities;
}
