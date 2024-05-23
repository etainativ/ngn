#include "engine/system.h"
#include "engine/time.h"
#include "entt/entity/fwd.hpp"
#include "networking/transport.h"
#include "networking/rpc.h"
#include "configuration/global.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"

#define MAX_INFLIGHT_TICKS 60

Client *client;
uint32_t __msgId = 10;

inFlightRPCMessages_t inFlightRPCMessages;
google::protobuf::Arena rpcArena;

void handleRPCMessage(const NetworkRPCMessage &rpc) {
    switch (rpc.networkRPCMessageType_case()) {
	case NetworkRPCMessage::kMessageAck:
	    if (inFlightRPCMessages.find(rpc.msgid()) != inFlightRPCMessages.end()) {
		delete inFlightRPCMessages[rpc.msgid()].rpc;
		inFlightRPCMessages.erase(rpc.msgid());
	    }
	    break;
	default:
	    // TODO log error
	    break;
    };
};


void readMessages() {
    Datagram data;
    while (clientRecv(client, &data)) {
	NetworkMessage msg;
	msg.ParseFromArray(data.data, data.size);
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


void sendMsg(NetworkMessage &msg) {
    size_t size = msg.ByteSizeLong();
    void *data = new char[size];
    if (msg.SerializeToArray(data, size)) {
	Datagram dg = { .size = size, .data = data };
       	clientSend(client, dg);
    }
}


void sendRPC(NetworkRPCMessage *rpc, uint32_t msgId) {
    rpc->set_msgid(msgId);
    inFlightRPCMessages[msgId] = {getCurrentTick(), rpc};
    NetworkMessage msg;
    msg.set_allocated_rpcmessage(rpc);
    sendMsg(msg);
    msg.release_rpcmessage();
}


void sendClientHello() {
    NetworkRPCMessage *newRPC = new NetworkRPCMessage();
    newRPC->mutable_clientconnect();
    sendRPC(newRPC, __msgId++);
}


void initClientNetworkSystem(entt::registry *entities) {
    client = clientInit(HOSTNAME, SERVER_PORT, 0);
    sendClientHello();
}


void retrySends() {
    for (auto &[msgId, msgInfo] : inFlightRPCMessages) {
	if ((getCurrentTick() - msgInfo.tickSent) > MAX_INFLIGHT_TICKS)
	    // TODO log
	    sendRPC(msgInfo.rpc, msgId);
    }
}


void updateClientNetworkSystem(entt::registry *entities) {
    readMessages();
    retrySends();
}


void destroyClientNetworkSystem(entt::registry *entities) {
    clientDestroy(client);
    // clean Messages in flight
}


struct System clientNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = initClientNetworkSystem,
    .update = updateClientNetworkSystem,
    .fixedUpdate = nullptr,
    .destroy = destroyClientNetworkSystem
};
