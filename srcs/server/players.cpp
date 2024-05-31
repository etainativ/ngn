#include "datatypes.h"
#include "server/networkid.h"
#include <map>


struct PlayersData {
    networkId_t networkid;
};


std::map<token_t, PlayersData> __playersMap;
void initPlayer(token_t) {
    networkId_t networkid = newNetworkId();
}
