#include "datatypes.h"


networkId_t __networkId = 0;


networkId_t newNetworkId() {
    return __networkId++;
}
