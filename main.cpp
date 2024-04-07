#include "engine/engine.h"
#include "engine/scene.h"
#include <glm/ext/matrix_transform.hpp>


int main() {
    Scene scene;
    GameObjects obj;
    scene.pipelines.push_back(new Pipeline());
    scene.pipelines[0]->vertexShaderFP = "../shaders/vert.gls";
    scene.pipelines[0]->fragShaderFP= "../shaders/frag.gls";

    obj.pipeline = scene.pipelines[0];
    obj.mesh.vertices.push_back(Vertex {glm::vec3(0.5f, -0.5f, 0.0f)});
    obj.mesh.vertices.push_back(Vertex {glm::vec3(0.5f, 0.5f, 0.0f)});
    obj.mesh.vertices.push_back(Vertex {glm::vec3(-0.5f, -0.5f, 0.0f)});
    obj.mesh.vertices.push_back(Vertex {glm::vec3(0.0f, 0.0f, -0.5f)});
    obj.mesh.indices.push_back(0);
    obj.mesh.indices.push_back(1);
    obj.mesh.indices.push_back(2);

    obj.mesh.indices.push_back(0);
    obj.mesh.indices.push_back(2);
    obj.mesh.indices.push_back(3);

    obj.mesh.indices.push_back(0);
    obj.mesh.indices.push_back(1);
    obj.mesh.indices.push_back(3);

    obj.mesh.indices.push_back(1);
    obj.mesh.indices.push_back(2);
    obj.mesh.indices.push_back(3);
    scene.objects.push_back(obj);

    scene.camera = glm::lookAt(
	    glm::vec3(0.0f, 0.0f, 1.0f),
	    glm::vec3(0.0f, 0.0f, 0.0f),
	    glm::vec3(0.0f, 1.0f, 0.0f));

    auto engine = Engine();
    engine.run(scene);
}
