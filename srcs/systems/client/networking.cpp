#include "engine/system.h"
#include "networking/transport.h"
#include "networking/rpc.h"
#include "configuration/global.h"
#include "time.h"

#define MAX_INFLISHGT_TIME 5 * CLOCKS_PER_SEC

Client *client;
msgId_t __msgId = 10;
struct inFlightRPCInfo {
    clock_t timeSent;
    Datagram dg;
};
std::map<msgId_t, inFlightRPCInfo> inFlightRPCMessages;


void handleRPCMessage(void *msg) {
    RPCBase *rpc = static_cast<RPCBase *>(msg);
    switch (rpc->rpcType) {
	case RPC_TYPE_SERVER_ACK:
	    if (inFlightRPCMessages.find(rpc->msgId) != inFlightRPCMessages.end()) {
		delete static_cast<RPCBase *>(inFlightRPCMessages[rpc->msgId].dg.data);
		inFlightRPCMessages.erase(rpc->msgId);
	    }
	    break;
	default:
	    // TODO log error
	    break;
    };
};


void sendRPCData(Datagram &data, msgId_t msgId) {
    clientSend(client, data);
    inFlightRPCMessages[msgId] = {clock(), data};
};


template <class T>
void sendRPC(T& rpc, msgId_t msgId) {
    assert(client != nullptr);
    T *msg = new T(rpc);
    Datagram data = {
	.size = sizeof(T),
	.data = msg
    };
    dynamic_cast<RPCBase*>(msg)->msgId = msgId;
    sendRPCData(data, msgId);
}


void initClientNetworkSystem(entt::registry *entities) {
    client = clientInit(HOSTNAME, SERVER_PORT, 0);
    RPCClientConnect rpc = {};
    sendRPC(rpc, __msgId++);
}



void readMessages() {
    Datagram data;
    while (clientRecv(client, &data)) {
	BaseMessage *msg = static_cast<BaseMessage *>(data.data);
	switch (msg->msgType) {
	    case MESSAGE_TYPE_RPC:
		handleRPCMessage(data.data);
		break;
	    case MESSAGE_TYPE_INPUTS:
		// TODO client should not get input MESSAGE_TYPE_SNAPSHOT
		// TODO log error
		break;
	    case MESSAGE_TYPE_SNAPSHOT:
		//handleSnapshotMessage(data.data);
		break;

	}

    }
}


void retrySends() {
    clock_t now = clock();
    for (auto &[msgId, msgInfo] : inFlightRPCMessages) {
	if ((now - msgInfo.timeSent) > MAX_INFLISHGT_TIME)
	    // TODO log
	    sendRPCData(msgInfo.dg, msgId);
    }
}

void updateClientNetworkSystem(entt::registry *entities) {
    readMessages();
    retrySends();
}


struct System ClientNetworkSystem = {
    .name = "NetworkSystem",
    .stats = {},
    .init = initClientNetworkSystem,
    .update = updateClientNetworkSystem,
    .fixedUpdate = nullptr,
    .destroy = nullptr
};
