#include "engine/scene.h"
#include "engine/system.h"

class Engine {
    public:
	void run(Scene &firstScene, std::vector<System *>& systems);
    private:
	void loadScene(Scene &scene);
	void unloadScene(Scene &scene);

};

