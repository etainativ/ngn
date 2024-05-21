#include "protobufs/rpc.pb.h"
#include "engine/time.h"


struct RPCMessageData {
    tick_t tickSent;
    NetworkRPCMessage *rpc;
};


typedef std::map<uint32_t, RPCMessageData> inFlightRPCMessages_t;
