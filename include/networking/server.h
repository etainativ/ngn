#pragma once
#include "protobufs/msgs.pb.h"
#include "datatypes.h"

void messageServerDestroy();
void messageServerInit(
	uint16_t port, int (*validator)(token_t token));

void sendRPC(
	NetworkRPCMessage*  rpc,
	token_t             token);

void sendMsg(
	NetworkMessage&  msg,
	token_t          token);

int recvMsg(
	NetworkMessage& msg,
	token_t*	token);

void serverRetryMsg();
