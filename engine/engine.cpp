#include "engine.h"
#include "systems.h"
#include "backends/imgui_impl_glfw.h"
#include "entt/entity/fwd.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>


void resizeCallback(GLFWwindow* window, int width, int height) {
    Engine *self = (Engine*)glfwGetWindowUserPointer(window);
    self->windowExtent = {(uint32_t)width, (uint32_t)height};
    self->resize();
}


Engine::Engine()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(
	    windowExtent.width, windowExtent.height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, &renderer);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    renderer = new Renderer(window);
}

Engine::~Engine() {
    delete renderer;
    glfwDestroyWindow(window);
    glfwTerminate();
}


void Engine::gameLoop() {
}


void Engine::run(Scene& scene)
{
    bool bQuit = false;

    entt::registry entities;
    loadScene(scene);
    struct SystemsData *systemsData = initSystems(scene, &entities);

    while (!bQuit)
    {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	    bQuit = true;

	glfwPollEvents();
	// gamelogic
	// imgui new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Render();
    }

    destroySystems(systemsData, &entities);
    unloadScene(scene);
}


void Engine::unloadScene(Scene &scene) {
    // wait for renderer to finish before destoying buffers and pipelines
    for (auto &pipeline : scene.pipelines) {
	renderer->unloadPipeline(pipeline);
    }

    for (auto &glft : scene.glftObjects) {
	GlftObject::unloadGlftObject(glft, renderer);
    }
};


void Engine::loadScene(Scene &scene) {
    for (auto &pipeline : scene.pipelines) {
	renderer->loadPipeline(pipeline);

    }

    for (auto &glft : scene.glftObjects) {
	GlftObject::loadGlftObject(glft, renderer);
    }
};

