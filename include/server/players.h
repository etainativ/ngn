#include "datatypes.h"
#include <map>

void initPlayer(token_t token);

struct PlayersData {
    networkId_t networkid;
};
extern std::map<token_t, PlayersData> __playersMap;
