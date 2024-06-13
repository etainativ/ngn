#include "engine/system.h"
#include "engine/entities.h"

#include "networking/client.h"
#include "networking/rpc.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"
#include "logging/logger.h"


void handleRPCMessage(const NetworkRPCMessage &rpc) {
    switch (rpc.networkRPCMessageType_case()) {
	case NetworkRPCMessage::kMessageAck:
	    DEBUG("Should not get ack messages here\n");
	    break;
	case NetworkRPCMessage::kClientNetworkID:
	    DEBUG("Client got networkID %d\n", rpc.clientnetworkid().netowrkid());
	    break;
	default:
	    // TODO log error
	    break;
    }
};


void readMessages() {
    NetworkMessage msg;
    while (recvMsg(msg)) {
	switch (msg.networkMessageType_case()) {
	    case NetworkMessage::kRpcMessage:
		handleRPCMessage(msg.rpcmessage());
		break;
	    case NetworkMessage::kInputMessage:
		// TODO client should not get input MESSAGE_TYPE_SNAPSHOT
		// TODO log error
		break;
	    case NetworkMessage::kSnapshotMessage:
		//handleSnapshotMessage(data.data);
		break;
	    case NetworkMessage::NETWORKMESSAGETYPE_NOT_SET:
		// TODO log error
		break;
	}
    }
}



void sendClientHello() {
    NetworkRPCMessage *newRPC = new NetworkRPCMessage();
    newRPC->mutable_clientconnect();
    sendRPC(newRPC);
}


void initClientNetworkSystem() {
    messageClientInit(0);
    sendClientHello();
}


void updateClientNetworkSystem() {
    readMessages();
    retryMsg();
}


void destroyClientNetworkSystem() {
    messageClientDestroy();
}


struct System clientNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = initClientNetworkSystem,
    .update = updateClientNetworkSystem,
    .fixedUpdate = nullptr,
    .destroy = destroyClientNetworkSystem
};
