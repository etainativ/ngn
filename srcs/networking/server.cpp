#include "networking/server.h"
#include "networking/transport.h"
#include "networking/rpc.h"
#include "protobufs/msgs.pb.h"
#include "protobufs/rpc.pb.h"
#include "datatypes.h"
#include "logging/logger.h"


typedef std::map<token_t, inFlightRPCMessages_t> inFlightRPC_t;

struct MessageServer {
    Server*        server;
    inFlightRPC_t  inFlightRPCMessages;
    uint32_t       msgId = 0;
} __server;


void messageServerInit(
	uint16_t port, int (*validator)(token_t token)) {
    __server.server = serverInit(port, validator);
}


void messageServerDestroy() {
    for (auto i : __server.inFlightRPCMessages) {
	for (auto j : i.second) {
	   delete j.second.rpc; 
	}
    }
    serverDestroy(__server.server);
}


void sendMsg(
	NetworkMessage&  msg,
	token_t          token) {
    size_t size = msg.ByteSizeLong();
    void *data  = new char[size];

    if (msg.SerializeToArray(data, size)) {
	Datagram dg = { .size = size, .data = data};
	serverSend(__server.server, dg, token);
    }
}


void __sendRPC(
	NetworkRPCMessage*  rpc,
	token_t             token) {
    __server.inFlightRPCMessages[token][rpc->msgid()] = {getCurrentTick(), rpc};
    NetworkMessage msg;
    msg.set_allocated_rpcmessage(rpc);
    sendMsg(msg, token);
    msg.release_rpcmessage();
}


void sendRPC(
	NetworkRPCMessage*  rpc,
	token_t             token) {
    rpc->set_msgid(__server.msgId++);
    __sendRPC(rpc, token);
}


void removeEntry(token_t token, uint32_t msgId) {
    if (__server.inFlightRPCMessages.find(token) == __server.inFlightRPCMessages.end()) {
	DEBUG("Message from unknown token %lu\n", token);
	return;
    }
    inFlightRPCMessages_t& rpcs = __server.inFlightRPCMessages[token];
    if  (rpcs.find(msgId) == rpcs.end()) {
	DEBUG("Ack For Unknown Message %u\n", msgId);
	return;
    }
    DEBUG("Earasing %u, %lu\n", msgId, token);
    delete rpcs[msgId].rpc;
    rpcs.erase(msgId);
}


void ackMessage(token_t token, uint32_t msgId) {
    NetworkMessage msg;
    NetworkRPCMessage *rpc = msg.mutable_rpcmessage();
    rpc->set_msgid(msgId);
    rpc->mutable_messageack();
    sendMsg(msg, token);
}


int recvMsg(
	NetworkMessage& msg,
	token_t*	token) {
    Datagram dg;
    while (serverRecv(__server.server, &dg, token)) {
	msg.ParseFromArray(dg.data, dg.size);
	// Seems like wrong place to deal with acks,
	// Maybe a change to the scheme is needed
	if (msg.has_rpcmessage()) {
	    const NetworkRPCMessage &rpc = msg.rpcmessage();
	    if (rpc.has_messageack()) {
		removeEntry(*token, rpc.msgid());
		continue;
	    }
	    ackMessage(*token, rpc.msgid());
	}
	return 1;
    }
    return 0;
}


void serverRetryMsg() {
    for (auto &[token, inFlightRPCs] : __server.inFlightRPCMessages) {
	for (auto &[msgId, msgInfo] : inFlightRPCs) {
	    if ((getCurrentTick() - msgInfo.tickSent) > MAX_INFLIGHT_TICKS) {
		DEBUG("Resending %u, %lu\n", msgId, token);
		__sendRPC(msgInfo.rpc, token);
	    }
	}
    }
}
