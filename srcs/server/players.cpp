#include "datatypes.h"
#include "protobufs/rpc.pb.h"
#include "server/networkid.h"
#include "networking/server.h"
#include <map>


struct PlayersData {
    networkId_t networkid;
};

std::map<token_t, PlayersData> __playersMap;

void createNewPlayer(token_t token) {
    networkId_t networkid = newNetworkId();
    __playersMap[token] = {
	.networkid = networkid };
}


networkId_t getNetworkID(token_t token) {
    if (__playersMap.find(token) == __playersMap.end())
	createNewPlayer(token);
    return __playersMap[token].networkid;
}


void initPlayer(token_t token) {
    networkId_t networkid = getNetworkID(token);
    NetworkRPCMessage *rpc = new NetworkRPCMessage;
    RPCClientNetworkID *clientNetworkID = rpc->mutable_clientnetworkid();
    clientNetworkID->set_netowrkid(networkid);
    sendRPC(rpc, token);
}
