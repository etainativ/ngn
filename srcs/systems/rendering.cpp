#include "systems/rendering.h"
#include "engine/rendering/renderer.h"

#include "entt/entity/fwd.hpp"
#include "backends/imgui_impl_glfw.h"


Renderer *__renderer = nullptr;
GLFWwindow* __window = nullptr;


void resizeCallback(GLFWwindow* window, int width, int height) {
    Renderer *self = (Renderer*)glfwGetWindowUserPointer(window);
    self->resize();
}


void renderingInit(entt::registry *entities) {
    VkExtent2D windowExtent = { 1200, 960 };
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    __window = glfwCreateWindow(
	    windowExtent.width, windowExtent.height,
	    "SWE", nullptr, nullptr);
    __renderer = new Renderer(__window);
    glfwSetWindowUserPointer(__window, __renderer);
    glfwSetFramebufferSizeCallback(__window, resizeCallback);
}


void renderingUpdate(entt::registry *entities) {
    std::vector<RenderData> bla;
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Render();
    __renderer->draw(bla);
}


void renderingDestroy(entt::registry *entities) {
    delete __renderer;
    glfwDestroyWindow(__window);
    glfwTerminate();
}


struct System renderingSystem = {
    .name = "rendering",
    .stats = {},
    .init = renderingInit,
    .update = renderingUpdate,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
