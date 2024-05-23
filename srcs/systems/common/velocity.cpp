#include "entt/entity/fwd.hpp"
#include "engine/system.h"
#include "engine/time.h"
#include "components/velocity.h"
#include "components/transform.h"

#include <glm/ext/matrix_transform.hpp>




void velocityFixedUpdate(entt::registry *entities) {
    // linear velocity
    {
	auto view = entities->view<velocity, transform>();
	view.each([](const auto &vel, auto &pos) {
		glm::vec3 deltaPos = vel.value * float(FIXED_UPDATE_INTERVAL);
		pos.value = glm::translate(glm::mat4(1.f), deltaPos) * pos.value; });
    }

    // angular velocity
    {
	auto view = entities->view<angularVelocity2D, transform>();
	view.each([](const auto &angular, auto &pos) {
		float angle = angular.value * float(FIXED_UPDATE_INTERVAL);
		pos.value = glm::rotate(pos.value, angle, {0.f, 0.f, 1.f});
		});
    }
}


System velocitySystem {
    .name = "velocity",
    .stats = {},
    .init = nullptr,
    .update = nullptr,
    .fixedUpdate = velocityFixedUpdate,
    .destroy = nullptr
};
