#include "engine/camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>


Camera camera = {
    .matrix = glm::mat4x4(1.f)
};


void updateCamera(Camera &c) {
    auto perspective = glm::perspective(
	    glm::radians(c.angle), c.ratio, c.near, c.far);
    perspective[1][1] *= -1.0f;

    auto lookat = glm::lookAt(c.pos, c.target, c.up);
    c.matrix = perspective * lookat;
}

