#include "engine/rendering/renderer.h"
#include "engine/scene.h"

class Engine {
    public:
	Engine();
	~Engine();
	VkExtent2D windowExtent = { 1200, 960 };
	void run(Scene &firstScene);
	void resize() {renderer->resize();}
    private:
	Renderer *renderer;
	GLFWwindow *window;

	void loadScene(Scene &scene);
	void unloadScene(Scene &scene);

};

