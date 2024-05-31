#include "engine/system.h"
#include "engine/time.h"
#include "components/tags.h"
#include "components/velocity.h"
#include "components/transform.h"

#include <entt/entity/fwd.hpp>
#include <GLFW/glfw3.h>
#include <glm/ext/quaternion_geometric.hpp>
#include <map>

// one minutes input buffer size, after that we are screwed
#define INPUT_BUFFER_SIZE FIXED_UPDATES_PER_SECOND * 60
extern bool isMainLoopRunning;
extern GLFWwindow* __window;

uint64_t inputBuffer[INPUT_BUFFER_SIZE] = {0};


// max keys should be 64 after
std::map<int, uint64_t> keysMapping {
    {GLFW_KEY_ESCAPE, 1},
    {GLFW_KEY_W, 1 << 1},
    {GLFW_KEY_A, 1 << 2},
    {GLFW_KEY_S, 1 << 3},
    {GLFW_KEY_D, 1 << 4},
};


uint64_t *getCurrentInput() {
    return &inputBuffer[getCurrentTick() % INPUT_BUFFER_SIZE];
}


void resetCurrentInput() {
    uint64_t *currentInput = getCurrentInput();
    *currentInput = 0;
}


void inputsSystemUpdate(entt::registry *entities) {
    uint64_t *currentInputs = getCurrentInput();
    for (auto key : keysMapping) {
	if (glfwGetKey(__window, key.first) == GLFW_PRESS) {
	    *currentInputs |= key.second;
	}
    }
}


void inputsSystemFixedUpdate(entt::registry *entities) {
    uint64_t currentInputs = *getCurrentInput();
    auto view = entities->view<playersEntity, transform, angularVelocity2D, velocity>();
    for (auto [tag, transform, angular, vel] : view.each()) {
	float delta = float(FIXED_UPDATE_INTERVAL);
	glm::vec3 up = glm::normalize(glm::vec3(
		-transform.value[0][0], // WHY????
		transform.value[1][0],
		transform.value[2][0]));
	if (currentInputs & keysMapping[GLFW_KEY_W]) {
	    vel.value += up * delta * 10.f;
	}
	if (currentInputs & keysMapping[GLFW_KEY_S]) {
	    vel.value -= up * delta * 10.f;
	}
	if (currentInputs & keysMapping[GLFW_KEY_A]) {
	    angular.value += delta;
	}
	if (currentInputs & keysMapping[GLFW_KEY_D]) {
	    angular.value -= delta;
	}
    }

    if (currentInputs & keysMapping[GLFW_KEY_ESCAPE])
	isMainLoopRunning = false;
    resetCurrentInput();

}

// depends on rendere system
System inputsSystem {
    .name = "velocity",
    .stats = {},
    .init = nullptr,
    .update = inputsSystemUpdate,
    .fixedUpdate = inputsSystemFixedUpdate,
    .destroy = nullptr
};
