#include "engine/system.h"
#include "networking/transport.h"
#include "configuration/global.h"

#include <iostream>

Server *networkServer;


int tokenValidator(token_t token) {
    return 0;
}

void initServerNetworkSystem(entt::registry *entities) {
    networkServer = serverInit(SERVER_PORT, tokenValidator);
    if (networkServer == nullptr) {
	std::cout << "Could not init serveer\n";
	abort();
    }
}

void updateServerNetworkSystem(entt::registry* entities) {
}

void destroyServerNetworkSystem(entt::registry* entities) {}


struct System ServerNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = initServerNetworkSystem,
    .update = updateServerNetworkSystem,
    .fixedUpdate = nullptr,
    .destroy = destroyServerNetworkSystem
};
