#include <glm/glm.hpp>

struct Camera {
    glm::vec3 pos;
    glm::vec3 target;
    glm::vec3 up;

    float angle;
    float ratio;
    float near;
    float far;

    glm::mat4x4 matrix;
};


void updateCamera(Camera &camera);
extern Camera camera;
