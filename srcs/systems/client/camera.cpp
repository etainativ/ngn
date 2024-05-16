#include "engine/system.h"
#include "engine/camera.h"

#include "components/translation.h"

#include <X11/X.h>
#include <entt/entt.hpp>
#include <GLFW/glfw3.h>
#include <glm/ext/quaternion_transform.hpp>
#include "glm/gtc/matrix_transform.hpp"


extern GLFWwindow* __window;
double pos[2];

#define POSX pos[0]
#define POSY pos[1]

void initCamera(entt::registry* entities) {
    camera.pos = glm::vec3(0.f, 0.f, 20.f);
    camera.target = glm::vec3(0.f, 0.f, 0.f);
    camera.up = glm::vec3(0.f, 1.f, 0.f);

    camera.ratio = 1.f;
    camera.angle = 120.f;
    camera.near = 0.1f;
    camera.far = 100.f;

    updateCamera(camera);
    glfwGetCursorPos(__window, &POSX, &POSY);
}


void cameraUpdate(entt::registry* entities) {
    double curX, curY, dx, dy;
    glfwGetCursorPos(__window, &curX, &curY);
    if (glfwGetMouseButton(__window, GLFW_MOUSE_BUTTON_1)) {
	dx = (curX - POSX) / 10.f;
	dy = (POSY - curY) / 10.f;
	camera.pos += glm::vec3(dx, dy, 0.f);
	camera.target += glm::vec3(dx, dy, 0.f);
	updateCamera(camera);
    }
    if (glfwGetMouseButton(__window, GLFW_MOUSE_BUTTON_2)) {
	float dx = (curX - POSX) / 100.f;
	float dy = (POSY - curY) / 100.f;
	glm::vec4 temp = glm::vec4(camera.pos, 0);
	temp = glm::rotate(glm::mat4(1.0f), dx, camera.up) * temp;
	camera.pos = { temp.x, temp.y, temp.z };
	updateCamera(camera);
    }
    POSX = curX;
    POSY = curY;
}


System cameraSystem {
    .name = "Camera System",
    .stats = {},
    .init = initCamera,
    .update = cameraUpdate,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};

