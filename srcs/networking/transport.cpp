#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>

#include "networking/transport.h"

// TODO authentication
// TODO avoid replay attack

// must limit data size for recv buffer size
// atbitrary choose 1MB
#define MAX_DATA_SIZE 1048576


struct MessageHeader {
    token_t token;
    msgsize_t dataSize;
};


struct Message {
    token_t token;
    msgsize_t dataSize;
    uint8_t *data;
};


struct Client {
    int fd;
    struct sockaddr_in serverAddr;
    token_t token;
    uint8_t recvBuffer[MAX_DATA_SIZE + sizeof(MessageHeader)];

    Client(int fd, struct sockaddr_in serverAddr, token_t token) :
	fd(fd), serverAddr(serverAddr), token(token) {};
};


struct Server {
    int fd;
    int (*validator)(token_t token);
    uint32_t msgid = 0;
    std::map<token_t, sockaddr_in> connectionMap;
    uint8_t recvBuffer[MAX_DATA_SIZE + sizeof(MessageHeader)];

    Server(int fd, int(*v)(token_t token)) : fd(fd), validator(v) {};
};


struct Client *clientInit(
	const char *hostname,
	uint16_t port,
	uint64_t token) {
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
	return nullptr;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(hostname);

    return new Client(fd, serverAddr, token);
}


struct Server *serverInit(uint16_t port, int (*validator)(token_t token)) {
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
	return nullptr;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // Bind to the set port and IP:
    if(bind(fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
	return nullptr;
    }

    return new Server(fd, validator);
}


int sendMesg(int fd, token_t token, sockaddr_in *address, struct Datagram data) {
    MessageHeader header = {};
    header.dataSize = data.size;
    header.token = token;

    iovec msgContext[2];
    msgContext[0].iov_base = &header;
    msgContext[0].iov_len = sizeof(MessageHeader);
    msgContext[1].iov_base = data.data;
    msgContext[1].iov_len = data.size;

    msghdr msghdr = {};
    msghdr.msg_name = address;
    msghdr.msg_namelen = sizeof(sockaddr_in);
    msghdr.msg_iov = msgContext;
    msghdr.msg_iovlen = 2;
    return sendmsg(fd, &msghdr, 0);
}


void clientSend(Client *client, struct Datagram data) {
    sendMesg(client->fd, client->token, &client->serverAddr, data);
}


void serverSend(Server *server, struct Datagram data, token_t token) {
    auto it = server->connectionMap.find(token);
    if (it == server->connectionMap.end()) {
	// TODO log!!!
	return;
    }

    sendMesg(server->fd, token, &it->second, data);
}

void serverSendAll(Server *server, struct Datagram data) {
    for (auto it : server->connectionMap)
	sendMesg(server->fd, it.first, &it.second, data);
}


int clientRecv(struct Client *client, struct Datagram *data) {
    struct MessageHeader *header = (struct MessageHeader *)client->recvBuffer;
    if (recvfrom(client->fd, client->recvBuffer, MAX_DATA_SIZE, MSG_DONTWAIT, nullptr, 0) <= 0) {
	data->data = nullptr;
	return 0;
    }

    // check token is valid??
    data->data = header + 1;
    data->size = header->dataSize;
    return 1;
}


int serverRecv(struct Server *server, struct Datagram *data, token_t *token) {
    sockaddr_in address;
    socklen_t len = sizeof(address);
    struct MessageHeader *header = (struct MessageHeader *)server->recvBuffer;
    while (recvfrom(server->fd, server->recvBuffer, MAX_DATA_SIZE, MSG_DONTWAIT,
		(struct sockaddr *)&address, &len) > 0) {

	// if message not valid we continue the loop and ignore this msg
	if (server->validator and !server->validator(header->token)) continue;

	server->connectionMap[header->token] = address;
	*token = header->token;
	data->size = header->dataSize;
	data->data = header + 1;
	return 0;
    }
    data->data = nullptr;
    return 1;
}


void clientDestroy(struct Client *client) {
    close(client->fd);
    delete client;
}
