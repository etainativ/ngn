#include "engine/system.h"


struct System NetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = nullptr,
    .update = nullptr,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
