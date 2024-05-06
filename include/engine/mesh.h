#pragma once
#include <glm/glm.hpp>


namespace Mesh {
    struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 color;
	float pad;
    };
}
