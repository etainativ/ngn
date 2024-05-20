#pragma once
#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "entt/entt.hpp"


struct RenderInitDataComponent {
    Pipeline::Pipeline* pipeline;
    GlftObject::GlftObject* glftObject;
};


struct renderable {
    entt::entity entity;
};


enum instanceType {
    SHIP
};


struct MakeInstanceComponent {
    enum instanceType type;
};
