#include "engine/system.h"
#include "networking/transport.h"
#include "configuration/global.h"


Client *client;

void initClientNetworkSystem(entt::registry *entities) {
    client = clientInit(HOSTNAME, SERVER_PORT, 0);
}

struct System ClientNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = nullptr,
    .update = nullptr,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
