#include "engine/engine.h"
#include "engine/systems.h"

#include "GLFW/glfw3.h"
extern GLFWwindow* __window;


void Engine::run(Scene& scene, std::vector<System *>& systems)
{
    bool bQuit = false;
    entt::registry entities;
    struct SystemsData *systemsData = createSystems(systems);
    initSystems(systemsData, &entities);
    while (!bQuit)
    {
	if (glfwGetKey(__window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	    bQuit = true;

	glfwPollEvents();
	// gamelogic
	// imgui new frame
	updateSystems(systemsData, &entities);
    }

    destroySystems(systemsData, &entities);
}
