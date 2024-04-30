#include "engine.h"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

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
    bool showDemoWindow = false;
    bool bQuit = false;
    
    renderer->loadScene(firstScene);
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

	//some imgui UI to test
	ImGui::ShowDemoWindow(&showDemoWindow);

	glm::mat4 x = firstScene.camera;
	ImGui::Text("%f %f %f %f", x[0][0], x[0][1], x[0][2], x[0][3]);
	ImGui::Text("%f %f %f %f", x[1][0], x[1][1], x[1][2], x[1][3]);
	ImGui::Text("%f %f %f %f", x[2][0], x[2][1], x[2][2], x[2][3]);
	ImGui::Text("%f %f %f %f", x[3][0], x[3][1], x[3][2], x[3][3]);
	ImGui::Render();
	renderer->draw(firstScene);
    }

    renderer->unloadScene(firstScene);
}
