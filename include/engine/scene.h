#pragma once
#include "pipeline.h"
#include "glft_object.h"
#include "entt/entt.hpp"
#include "engine/systems.h"

struct Scene {
    std::vector<System *>systems;
    std::vector<Pipeline::Pipeline> pipelines;
    std::vector<GlftObject::GlftObject> glftObjects;
    entt::registry sceneRegistry;
};
