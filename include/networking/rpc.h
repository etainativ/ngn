#include "protobufs/rpc.pb.h"
#include "engine/time.h"


#define MAX_INFLIGHT_TICKS 60


struct RPCMessageData {
    tick_t tickSent;
    NetworkRPCMessage *rpc;
};


typedef std::map<uint32_t, RPCMessageData> inFlightRPCMessages_t;
