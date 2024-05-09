#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/system.h"
#include "components/render_comp.h"


entt::entity __intancerShipEntity;



void instancerInit(entt::registry *entities) {
    __intancerShipEntity = entities->create();

    GlftObject::GlftObject* shipsGlftObject =
	new GlftObject::GlftObject("models/ship.gltf");

    Pipeline::Pipeline* shipsPipeline =
	new Pipeline::Pipeline(
		"shaders/vert.gls",
		"shaders/frag.gls");

    entities->emplace<RenderComponentInitData>(
	    __intancerShipEntity, shipsPipeline, shipsGlftObject);

}


void instancerUpdate(entt::registry *enetities) {
}


System instancerSystem {
    .name = "instancer",
    .stats = {},
    .init = instancerInit,
    .update = instancerUpdate,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
