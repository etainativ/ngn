#include "engine/system.h"
#include "networking/client.h"
#include "server/players.h"
#include "networking/server.h"
#include "networking/rpc.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"
#include "logging/logger.h"
#include "configuration/global.h"


void initServerNetworkSystem() {
    messageServerInit(SERVER_PORT, nullptr);
}


void handleRPCMessage(const NetworkRPCMessage &rpc, token_t token) {
    switch (rpc.networkRPCMessageType_case()) {
	case NetworkRPCMessage::kMessageAck:
	    DEBUG("this is an error");
	    break;
	case NetworkRPCMessage::kClientConnect:
	    DEBUG("Player connected msgId %d\n", rpc.msgid());
	    initPlayer(token);
	    break;
	default:
	    // TODO log error
	    break;
    };
}


void readMessages() {
    NetworkMessage msg;
    token_t token;
    while (recvMsg(msg, &token)) {
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


void updateServerNetworkSystem() {
    readMessages();
    serverRetryMsg();
}


void destroyServerNetworkSystem() {
    messageServerDestroy();
}


struct System serverNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = initServerNetworkSystem,
    .update = updateServerNetworkSystem,
    .fixedUpdate = nullptr,
    .destroy = destroyServerNetworkSystem
};
