#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/system.h"
#include "entt/entity/fwd.hpp"
#include "components/instancer.h"
#include "components/translation.h"


entt::entity __intancerShipEntity;


void createEntity(entt::registry *entities, instanceType type) {
    switch (type) {
	case SHIP:
	    auto e = entities->create();
	    entities->emplace<renderable>(e, __intancerShipEntity);
	    entities->emplace<translation>(e, glm::mat4x4(1.f));
	    break;
    }
}

void instancerInit(entt::registry *entities) {
    __intancerShipEntity = entities->create();

    GlftObject::GlftObject* shipsGlftObject =
	new GlftObject::GlftObject("models/ship.gltf");

    Pipeline::Pipeline* shipsPipeline =
	new Pipeline::Pipeline(
		"shaders/vert.gls",
		"shaders/frag.gls");

    entities->emplace<RenderInitDataComponent>(
	    __intancerShipEntity, shipsPipeline, shipsGlftObject);


    auto firstInstance = entities->create();
    entities->emplace<MakeInstanceComponent>(
	    firstInstance, instanceType::SHIP);

}


void instancerUpdate(entt::registry *entities) {
   auto view = entities->view<MakeInstanceComponent>();
   for (const entt::entity e : view) {
       auto &data = view.get<MakeInstanceComponent>(e);
       createEntity(entities, data.type);
       entities->destroy(e);

   }
}


System instancerSystem {
    .name = "instancer",
    .stats = {},
    .init = instancerInit,
    .update = instancerUpdate,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};