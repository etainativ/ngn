#pragma once
#include "pipeline.h"
#include "vk_types.h"
#include <glm/glm.hpp>


struct Vertex {
    glm::vec3 pos;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indicesBuffer;
};

struct GameObjects {
    glm::mat4 translation;
    Mesh mesh;
    Pipeline *pipeline;
};

struct Scene {
    std::vector<Pipeline*> pipelines;
    std::vector<GameObjects> objects;
    glm::mat4 camera;
};
