#pragma once
#include "engine/rendering/renderer.h"
#include <vector>
#include <string>

namespace GlftObject {
    struct GlftMesh {
	uint32_t firstIndex;
	uint32_t indexCount;
	int materialIndex;

    };

    struct GlftNode {
	std::vector<GlftNode> children;
	std::vector<GlftMesh> meshes;
	glm::mat4x4 transform;

    };

    struct GlftObject {
	std::string filename;
	std::vector<GlftNode> children;
	RendererMeshData *meshData;
    };

    void loadGlftObject(GlftObject& glftObject, Renderer *renderer);
    void unloadGlftObject(GlftObject& glftObject, Renderer *renderer);
}
