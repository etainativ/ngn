#include "engine/glft_object.h"
#include "rendering/renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace GlftObject {
    int getAccessorIndex(tinygltf::Primitive &primitive, const std::string attrName) {
	if (primitive.attributes.find(attrName) == primitive.attributes.end())
	    return -1;

	return primitive.attributes.find(attrName)->second;
    }

    const float *getPrimitiveAttribute(
	    tinygltf::Model& model,
	    tinygltf::Primitive& primitive,
	    const std::string attrName) {

	int accessorIndex = getAccessorIndex(primitive, attrName);
	if (accessorIndex == -1) return nullptr;

	const tinygltf::Accessor& accessor = model.accessors[accessorIndex];
	const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
	size_t offset = accessor.byteOffset + view.byteOffset;
	return reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[offset]));
    }

    GlftNode parseNode(
	    tinygltf::Model& model,
	    tinygltf::Node& inputNode,
	    std::vector<Mesh::Vertex>& vertices,
	    std::vector<uint32_t>& indices) {
	GlftNode node;
	node.transform = glm::mat4(1.0f);

	// Get the local node matrix
	// It's either made up from translation, rotation, scale or a 4x4 matrix
	if (inputNode.translation.size() == 3) {
	    node.transform = glm::translate(node.transform, glm::vec3(glm::make_vec3(inputNode.translation.data())));
	}
	if (inputNode.rotation.size() == 4) {
	    glm::quat q = glm::make_quat(inputNode.rotation.data());
	    node.transform *= glm::mat4(q);
	}
	if (inputNode.scale.size() == 3) {
	    node.transform = glm::scale(node.transform, glm::vec3(glm::make_vec3(inputNode.scale.data())));
	}
	if (inputNode.matrix.size() == 16) {
	    node.transform = glm::make_mat4x4(inputNode.matrix.data());
	};

	// load mesh data
	if (inputNode.mesh > -1) {
	    tinygltf::Mesh mesh = model.meshes[inputNode.mesh];
	    for (tinygltf::Primitive& primitive : mesh.primitives) {
		uint32_t firstIndex = static_cast<uint32_t>(indices.size());
		uint32_t vertexStart = static_cast<uint32_t>(vertices.size());
		uint32_t indexCount = 0;
		// Loading vertices data
		{
		    const float *pos = getPrimitiveAttribute(model, primitive, "POSITION");
		    const float *norm = getPrimitiveAttribute(model, primitive, "NORMAL");
		    const float *uv = getPrimitiveAttribute(model, primitive, "TEXCOORD_0");

		    if (pos == nullptr) continue;
		    int accessorIndex = getAccessorIndex(primitive, "POSITION");
		    size_t vertexCount = model.accessors[accessorIndex].count;

		    // Append data to model's vertex buffer
		    for (size_t v = 0; v < vertexCount; v++) {
			Mesh::Vertex vert{};
			vert.pos = glm::vec4(glm::make_vec3(&pos[v * 3]), 1.0f);
			vert.normal = glm::normalize(glm::vec3(norm ? glm::make_vec3(&norm[v * 3]) : glm::vec3(0.0f)));
			vert.uv = uv ? glm::make_vec2(&uv[v * 2]) : glm::vec3(0.0f);
			vert.color = glm::vec3(1.0f);
			vertices.push_back(vert);
		    }
		}
		// loding indices data
		{
		    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
		    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

		    indexCount = static_cast<uint32_t>(accessor.count);

		    // glTF supports different component types of indices
		    switch (accessor.componentType) {
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: 
			    {
				const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
				    indices.push_back(buf[index] + vertexStart);
				}
				break;
			    }
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: 
			    {
				const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
				    indices.push_back(buf[index] + vertexStart);
				}
				break;
			    }
			case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
			    {
				const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
				for (size_t index = 0; index < accessor.count; index++) {
				    indices.push_back(buf[index] + vertexStart);
				}
				break;
			    }
			default:
			    throw std::runtime_error("Invalid GLFT Indices Format");
		    }
		}
		GlftMesh mesh{};
		mesh.firstIndex = firstIndex;
		mesh.indexCount = indexCount;
		mesh.materialIndex = primitive.material;
		node.meshes.push_back(mesh);
	    }
	}

	// Load node's children
	if (inputNode.children.size() > 0) {
	    for (int child : inputNode.children) {
		node.children.push_back(
			parseNode(
			    model,
			    model.nodes[child],
			    vertices,
			    indices));
	    }
	}
	return node;
    }

    void loadGlftObject(GlftObject& glftObject, Renderer *renderer) {
	

	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;

	std::vector<Mesh::Vertex> vertices;
	std::vector<uint32_t> indices;

	if (not loader.LoadASCIIFromFile(&model, &err, &warn, glftObject.filename)) {
	    throw std::runtime_error(err);
	}

	if (model.scenes.size() < 1) {
	    // TODO better error
	    throw std::runtime_error(glftObject.filename);
	}
	// ignoring all over scenes
	for (int nodeIndex : model.scenes[0].nodes) {
	    glftObject.children.push_back(
		    parseNode(
			model,
			model.nodes[nodeIndex],
			vertices,
			indices));
	}

	glftObject.meshData = renderer->loadMesh(vertices, indices);
    };

    void unloadGlftObject(GlftObject& glftObject, Renderer *renderer) {
	renderer->unloadMesh(glftObject.meshData);
    }
}
