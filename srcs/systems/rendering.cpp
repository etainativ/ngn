#include "engine/system.h"
#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/rendering/renderer.h"
#include "components/render_comp.h"

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

    std::set<Pipeline::Pipeline*> pipelines;
    auto view = entities->view<RenderComponentInitData>();
    for (const entt::entity e : view) {
	RenderComponentInitData &data = view.get<RenderComponentInitData>(e);
	pipelines.insert(data.pipeline);
	GlftObject::loadGlftObject(*data.glftObject, __renderer);
    }

    for (auto pipeline : pipelines)
	__renderer->loadPipeline(*pipeline);
}


void renderingUpdate(entt::registry *entities) {
    std::vector<RenderData> bla;
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Render();
    __renderer->draw(bla);
}


void renderingDestroy(entt::registry *entities) {
    std::set<Pipeline::Pipeline*> pipelines;
    auto view = entities->view<RenderComponentInitData>();
    for (const entt::entity e : view) {
	RenderComponentInitData &data = view.get<RenderComponentInitData>(e);
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
