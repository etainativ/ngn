#include "engine/time.h"
#include "networking/client.h"
#include "networking/transport.h"
#include "networking/rpc.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"
#include "configuration/global.h"
#include "datatypes.h"


struct MessageClientData {
    Client*                client;
    msgId_t                msgId = 0;
    inFlightRPCMessages_t  inFlightRPCMessages;
} __client;



void ackMessage(msgId_t msgid) {
    NetworkMessage msg;
    NetworkRPCMessage *rpc = msg.mutable_rpcmessage();
    rpc->set_msgid(msgid);
    rpc->mutable_messageack();
    sendMsg(msg);
};


void messageClientInit(token_t token) {
    __client.client = clientInit(HOSTNAME, SERVER_PORT, token);
}


void messageClientDestroy() {
    for (auto i : __client.inFlightRPCMessages) {
	delete i.second.rpc;
	__client.inFlightRPCMessages.erase(i.first);
    }
    clientDestroy(__client.client);
}


void ackedMessage(msgId_t msgid) {
    if (__client.inFlightRPCMessages.find(msgid) == __client.inFlightRPCMessages.end()) return;

    delete __client.inFlightRPCMessages[msgid].rpc;
    __client.inFlightRPCMessages.erase(msgid);
}


void __sendRPC(NetworkRPCMessage* rpc) {
    __client.inFlightRPCMessages[rpc->msgid()] = {getCurrentTick(), rpc};
    NetworkMessage msg;
    msg.set_allocated_rpcmessage(rpc);
    sendMsg(msg);
    msg.release_rpcmessage();
}


void sendRPC(NetworkRPCMessage* rpc) {
    msgId_t msgId = __client.msgId++;
    rpc->set_msgid(msgId);
    __sendRPC(rpc);
}


void sendMsg(NetworkMessage& msg) {
    size_t size = msg.ByteSizeLong();
    void *data = new char[size];
    if (msg.SerializeToArray(data, size)) {
	Datagram dg = { .size = size, .data = data };
	clientSend(__client.client, dg);
    }
}


int recvMsg(NetworkMessage& msg) {
    Datagram data;
    while (clientRecv(__client.client, &data)) {
	msg.ParseFromArray(data.data, data.size);
	if (msg.has_rpcmessage()) {
	    // Seems like wrong place to deal with acks,
	    // Maybe a change to the scheme is needed
	    const NetworkRPCMessage& rpc = msg.rpcmessage();
	    msgId_t msgId = rpc.msgid();
	    if (rpc.has_messageack()) {
		ackedMessage(msgId);
		continue;
	    }
	    ackMessage(msgId);
	}
	return 1;
    }
    return 0;
}


void retryMsg() {
    for (auto &[msgId, msgInfo] : __client.inFlightRPCMessages) {
	if ((getCurrentTick() - msgInfo.tickSent) > MAX_INFLIGHT_TICKS)
	    // TODO log
	    __sendRPC(msgInfo.rpc);
    }
}
