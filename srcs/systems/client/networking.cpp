#include "engine/system.h"


struct System ClientNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = nullptr,
    .update = nullptr,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
