#include "engine/engine.h"
#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/scene.h"
#include <glm/ext/matrix_transform.hpp>

//systems
#include "systems/rendering.h"

int main() {
    Scene scene;
    scene.systems.push_back(&renderingSystem);
    scene.pipelines.push_back(Pipeline::initPipelineStruct(
		"../shaders/vert.gls",
		"../shaders/frag.gls"));
    scene.glftObjects.push_back(GlftObject::GlftObject {
	    .filename = "../models/ship.gltf"});

    auto engine = Engine();
    engine.run(scene);
}
