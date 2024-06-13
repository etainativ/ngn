#include "engine/system.h"
#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/camera.h"
#include "engine/entities.h"
#include "rendering/renderer.h"

#include "components/instancer.h"
#include "components/transform.h"
#include "components/velocity.h" // Remove later
#include "components/tags.h" // Remove later

#include <entt/entity/fwd.hpp>
#include <backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>


Renderer *__renderer = nullptr;
GLFWwindow* __window = nullptr;


void resizeCallback(GLFWwindow* window, int width, int height) {
    Renderer *self = (Renderer*)glfwGetWindowUserPointer(window);
    self->resize();
}


void renderingInit() {
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


void imGuiDraw() {
    auto view = entities->view<playersEntity, velocity, transform>();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    view.each([](const auto &tag, const auto &vel, const auto &pos){
	    glm::mat4 p = pos.value;
	    ImGui::Text("%f %f %f %f", p[0][0], p[0][1], p[0][2], p[0][3]);
	    ImGui::Text("%f %f %f %f", p[1][0], p[1][1], p[1][2], p[1][3]);
	    ImGui::Text("%f %f %f %f", p[2][0], p[2][1], p[2][2], p[2][3]);
	    ImGui::Text("%f %f %f %f", p[3][0], p[3][1], p[3][2], p[3][3]);
	    ImGui::Text("%f %f %f", vel.value[0], vel.value[1], vel.value[2]);
	    });
    ImGui::Text("Pos %f %f %f", camera.pos[0], camera.pos[1], camera.pos[2]);
    ImGui::Text("Target %f %f %f", camera.target[0], camera.target[1], camera.target[2]);
    ImGui::Text("Up %f %f %f", camera.up[0], camera.up[1], camera.up[2]);
    ImGui::EndFrame();
    ImGui::Render();
}


void renderingUpdate() {
    imGuiDraw();
    VkCommandBuffer cmd = __renderer->startDraw();
    for(auto &&[entity, rend, pos]: entities->view<renderable, transform>().each()) {
	auto &renderData = entities->get<RenderInitDataComponent>(rend.entity);
	Pipeline::Pipeline *pipeline = renderData.pipeline;
	GlftObject::GlftObject *glftObj = renderData.glftObject;
	__renderer->draw(cmd, pipeline, glftObj, camera.matrix * pos.value);
    }

    __renderer->finishDraw(cmd);

    glfwPollEvents();
}


void renderingDestroy() {
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
