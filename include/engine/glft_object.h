#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Renderer;
struct RendererMeshData;
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
	GlftObject(std::string filename) : filename(filename){};
    };

    void loadGlftObject(GlftObject& glftObject, Renderer *renderer);
    void unloadGlftObject(GlftObject& glftObject, Renderer *renderer);
}
