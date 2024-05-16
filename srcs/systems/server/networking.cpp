#include "engine/system.h"


struct System ServerNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = nullptr,
    .update = nullptr,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
