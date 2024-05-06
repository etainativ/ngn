#include "engine.h"
#include "backends/imgui_impl_glfw.h"
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


void Engine::run(Scene &firstScene)
{
    //bool showDemoWindow = false;
    bool bQuit = false;

    loadScene(firstScene);
    std::vector<RenderData> renderData;

    glm::vec3 eye {0.f, 0.f, 10.0f};
    glm::vec3 center {0.f, 0.f, 0.f};
    glm::vec3 up {0.f, 1.f, 0.f};
    renderData.push_back(RenderData {
	    .transform = glm::lookAt(eye, center, up),
	    .pipeline = &(firstScene.pipelines[0]),
	    .data = firstScene.glftObjects[0].meshData,
	    .indicesCount = firstScene.glftObjects[0].children[0].meshes[0].indexCount
	    });
    /*
    renderData[0].transform = glm::mat4x4(
	    1.f, 0.f, 0.f, 0.f,
	    0.f, 1.f, 0.f, 0.f,
	    0.f, 0.f, 1.f, 1.f,
	    0.f, 0.f, 0.f, 1.f);
	    */

    //renderData[0].transform *=  glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 256.0f)
    renderData[0].transform = glm::perspectiveRH_NO(glm::radians(120.0f), 1.f, 0.3f, 256.0f) * renderData[0].transform;
    float angle = 0;
    while (!bQuit)
    {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	    bQuit = true;

	// gamelogic
	glfwPollEvents();
	// imgui new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// setting camera
	/*
	ImGui::SliderFloat3("eye", &eye.x, -100, 100);
	ImGui::SliderFloat3("center", &center.x, -100, 100);
	ImGui::SliderFloat3("up", &up.x, -100, 100);

	ImGui::InputFloat3("eye", &eye.x);
	ImGui::InputFloat3("center", &center.x);
	ImGui::InputFloat3("up", &up.x);

	//
	renderData[0].transform = glm::lookAt(eye, center, up);
	*/
	glm::mat4x4 x = renderData[0].transform;
	ImGui::Text("%f %f %f %f", x[0][0], x[0][1], x[0][2], x[0][3]);
	ImGui::Text("%f %f %f %f", x[1][0], x[1][1], x[1][2], x[1][3]);
	ImGui::Text("%f %f %f %f", x[2][0], x[2][1], x[2][2], x[2][3]);
	ImGui::Text("%f %f %f %f", x[3][0], x[3][1], x[3][2], x[3][3]);
	renderData[0].transform = x * glm::rotate(glm::mat4x4(1.f), angle, glm::normalize(glm::vec3(0.f, 1.f, 0.f)));
	//ImGui::ShowDemoWindow(&showDemoWindow);
	ImGui::Render();
	renderer->draw(renderData);
	angle += 0.0001;
    }

    unloadScene(firstScene);
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

