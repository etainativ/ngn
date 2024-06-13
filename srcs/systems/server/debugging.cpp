#include "engine/system.h"
#include "server/players.h"
#include "protobufs/debugging.pb.h"

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <list>


int debugSocketFD = -1;
const int enable = 1;
const char *SOCKET_PATH = "/tmp/debugging.sock";


std::list<int> DebugServerClientsFD;
std::list<int> DebugServerClientsFDClosed;

void initServerDebugPort() {
    debugSocketFD = socket(AF_UNIX, SOCK_STREAM, 0);
    assert(debugSocketFD >= 0);

    assert(setsockopt(
		debugSocketFD,
		SOL_SOCKET,
		SO_REUSEADDR,
		&enable,
		sizeof(int)) == 0);
    struct sockaddr_un {
	sa_family_t sun_family;
	char sun_path[108];
    } addr;

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    unlink(SOCKET_PATH);
    int res = bind(
	    debugSocketFD, (struct sockaddr*)&addr, sizeof(addr));
    assert(res == 0);

    res = listen(debugSocketFD, 5);
    assert(res == 0);
    fcntl(debugSocketFD, F_SETFL, O_NONBLOCK);
}


void serverDebugSend(DebugMessage &msg, int fd) {
    size_t size = msg.ByteSizeLong();
    uint8_t *buf = new uint8_t[size];

    msg.SerializeToArray(buf, size);
    send(fd, buf, size, 0);
    delete[] buf;
}

void serverDebugSendError(int fd) {
    DebugMessage msg;
    DebugResponseMessage *resp = msg.mutable_debugresponsemessage();
    resp->mutable_debugresponseerror();
    serverDebugSend(msg, fd);
}


void serverDebugSendPlayerList(int fd) {
    DebugMessage msg;
    DebugResponseMessage *resp = msg.mutable_debugresponsemessage();
    DebugResponseListPlayersMessage *players = resp->mutable_debugresponselistplayersmessage();

    for (auto &player : __playersMap) {
	auto entry = players->add_players();
	entry->set_token(player.first);
	entry->set_networkid(player.second.networkid);
    }

    serverDebugSend(msg, fd);
}

void updateServerDebugPort() {
    char buffer[1024];

    int clientFD = accept(debugSocketFD, nullptr, nullptr);
    while (clientFD > 0) {
	DebugServerClientsFD.push_back(clientFD);
    	clientFD = accept(debugSocketFD, nullptr, nullptr);
    }

    for (int clientFD : DebugServerClientsFD) {
	int respSize = recv(clientFD, buffer, sizeof(buffer), MSG_DONTWAIT);
	if (respSize <= 0) continue;

	DebugMessage msg;
	if (msg.ParseFromArray(buffer, respSize) != true) {
	    std::cout << "Failed to parse message" << std::endl;
	    serverDebugSendError(clientFD);
	    close(clientFD);
	    DebugServerClientsFDClosed.push_back(clientFD);
	    continue;
	}

	if (msg.debugMessage_case() != DebugMessage::kDebugRequestMessage) {
	    std::cout << "Received message that is not a request" << std::endl;
	    serverDebugSendError(clientFD);
	    close(clientFD);
	    DebugServerClientsFDClosed.push_back(clientFD);
	    continue;
	}


	DebugRequestMessage req = msg.debugrequestmessage();
	switch (req.debugRequestMessage_case()) {
	    case DebugRequestMessage::kDebugRequestListPlayersMessage:
		serverDebugSendPlayerList(clientFD);
		break;
	    default:
		break;
	}

	close(clientFD);
	DebugServerClientsFDClosed.push_back(clientFD);
    }

    for (int clientFD : DebugServerClientsFDClosed) {
	DebugServerClientsFD.remove(clientFD);
    }
    DebugServerClientsFDClosed.clear();
}


void destroyServerDebugPort() {
    close(debugSocketFD);
    unlink(SOCKET_PATH);
}

struct System serverDebugPort = {
    .name = "ServerDebugPort",
    .stats = {},
    .init = initServerDebugPort,
    .update = updateServerDebugPort,
    .fixedUpdate = nullptr,
    .destroy = destroyServerDebugPort
};
