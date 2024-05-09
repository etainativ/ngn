#pragma once
#include "engine/glft_object.h"
#include "engine/pipeline.h"

struct RenderComponentInitData {
    Pipeline::Pipeline* pipeline;
    GlftObject::GlftObject* glftObject;
};
