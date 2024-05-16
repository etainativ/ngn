#include "GLFW/glfw3.h"
#include "engine/system.h"
#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/rendering/renderer.h"
#include "engine/camera.h"

#include "components/instancer.h"
#include "components/translation.h"

#include "entt/entity/fwd.hpp"
#include "backends/imgui_impl_glfw.h"



Renderer *__renderer = nullptr;
GLFWwindow* __window = nullptr;
extern bool isMainLoopRunning;


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

    std::set<Pipeline::Pipeline*> pipelines;
    auto view = entities->view<RenderInitDataComponent>();
    for (const entt::entity e : view) {
	auto &data = view.get<RenderInitDataComponent>(e);
	pipelines.insert(data.pipeline);
	GlftObject::loadGlftObject(*data.glftObject, __renderer);
    }

    for (auto pipeline : pipelines)
	__renderer->loadPipeline(*pipeline);
}


void renderingUpdate(entt::registry *entities) {
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Text("Pos %f %f %f", camera.pos[0], camera.pos[1], camera.pos[2]);
    ImGui::Text("Target %f %f %f", camera.target[0], camera.target[1], camera.target[2]);
    ImGui::Text("Up %f %f %f", camera.up[0], camera.up[1], camera.up[2]);
    ImGui::Render();
    VkCommandBuffer cmd = __renderer->startDraw();
    for(auto &&[entity, rend, pos]: entities->view<renderable, translation>().each()) {
	auto &renderData = entities->get<RenderInitDataComponent>(rend.entity);
	Pipeline::Pipeline *pipeline = renderData.pipeline;
	GlftObject::GlftObject *glftObj = renderData.glftObject;
	__renderer->draw(cmd, pipeline, glftObj, camera.matrix * pos.value);
    }

    __renderer->finishDraw(cmd);

    if (glfwGetKey(__window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	isMainLoopRunning = false;
    glfwPollEvents();
}


void renderingDestroy(entt::registry *entities) {
    std::set<Pipeline::Pipeline*> pipelines;
    auto view = entities->view<RenderInitDataComponent>();
    for (const entt::entity e : view) {
	RenderInitDataComponent &data = view.get<RenderInitDataComponent>(e);
	pipelines.insert(data.pipeline);
	GlftObject::unloadGlftObject(*data.glftObject, __renderer);
    }

    for (auto pipeline : pipelines)
	__renderer->unloadPipeline(*pipeline);

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
