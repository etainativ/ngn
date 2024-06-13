#pragma once
#include "protobufs/msgs.pb.h"
#include "datatypes.h"

void messageClientDestroy();
void messageClientInit(token_t token);
void sendRPC(NetworkRPCMessage* rpc);
void sendMsg(NetworkMessage& msg);
int recvMsg(NetworkMessage& msg);
void retryMsg();
