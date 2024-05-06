#pragma once
#include "pipeline.h"
#include "glft_object.h"
#include "entt/entt.hpp"

struct Scene {
    std::vector<Pipeline::Pipeline> pipelines;
    std::vector<GlftObject::GlftObject> glftObjects;
    entt::registry sceneRegistry;
};
