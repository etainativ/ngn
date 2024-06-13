#include "datatypes.h"


networkId_t __networkId = 1;


networkId_t newNetworkId() {
    return __networkId++;
}
