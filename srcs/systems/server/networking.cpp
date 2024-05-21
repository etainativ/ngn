#include "engine/system.h"
#include "networking/transport.h"
#include "configuration/global.h"
#include "networking/rpc.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"


Server *networkServer;
typedef std::map<token_t, inFlightRPCMessages_t> serverInFlightRPCMessages_t;
serverInFlightRPCMessages_t inFlightRPCMessages;

int tokenValidator(token_t token) {
    return 0;
}

void initServerNetworkSystem(entt::registry *entities) {
    networkServer = serverInit(SERVER_PORT, tokenValidator);
    if (networkServer == nullptr) {
	// TODO log
	abort();
    }
}


void removeEntry(token_t token, uint32_t msgId) {
    if (inFlightRPCMessages.find(token) == inFlightRPCMessages.end()) {
	//log??
	return;
    }
    inFlightRPCMessages_t clientRPCs = inFlightRPCMessages[token];
    if (clientRPCs.find(msgId) == clientRPCs.end()) {
	// log??
	return;
    }
    delete clientRPCs[msgId].rpc;
    clientRPCs.erase(msgId);
}


void initPlayer(token_t token) {
}


void handleRPCMessage(const NetworkRPCMessage &rpc, token_t token) {
    switch (rpc.networkRPCMessageType_case()) {
	case NetworkRPCMessage::kMessageAck:
	    removeEntry(token, rpc.msgid());
	    break;
	case NetworkRPCMessage::kClientConnect:
	    initPlayer(token);
	    break;
	default:
	    // TODO log error
	    break;
    };
}


void updateServerNetworkSystem(entt::registry* entities) {
    Datagram data;
    token_t token;
    while (serverRecv(networkServer, &data, &token)) {
	NetworkMessage msg;
	msg.ParseFromArray(data.data, data.size);
	switch (msg.networkMessageType_case()) {
	    case NetworkMessage::kRpcMessage:
		handleRPCMessage(msg.rpcmessage(), token);
		break;
	    case NetworkMessage::kInputMessage:
		//handleInpueMessage(msg.inputmessage());
		break;
	    case NetworkMessage::kSnapshotMessage:
		// TODO log error
		break;
	    case NetworkMessage::NETWORKMESSAGETYPE_NOT_SET:
		// TODO log error
		break;
	}
    }
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
