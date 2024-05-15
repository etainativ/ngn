#include "networking/transport.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// TODO automate tests
void startServer() {
    printf("Starting server \n");
    struct Server *server = serverInit(1234, nullptr);
    if (server == nullptr) {
	printf("Failed to init server\n");
	exit(1);
    }
    while (1) {
	struct Datagram data;
	token_t token = 4321;
	data.data = (void*)"1234";
	data.size = 4;
	serverUpdate(server);
	serverRecv(server, &data, &token);
	if (data.data) {
	    for (int x=0; x<data.size; x++) {
		printf("%c", ((char *)data.data)[x]);
	    }
	    printf("\n");
	    serverSend(server, data, token);
	    dataRelease(data);
	}
    }
}

void startClient() {
    printf("Starting client \n");
    struct Client *client = clientInit("127.0.0.1", 1234, 127837);
    while (1) {
	char msg[30] = {};
	clientUpdate(client);
	scanf("%s", &msg);
	struct Datagram data;
	data.data = msg;
	data.size = 30;
	clientSend(client, data);
	clientUpdate(client);
	clientRecv(client, &data);
	if (data.data) {
	    for (int x=0; x<data.size; x++) {
		printf("%c", ((char *)data.data)[x]);
	    }
	    printf("\n");
	    dataRelease(data);
	}
	
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
	printf("This program only get 1 argument, got %d\n", argc);
	exit(1);
    }

    if (strcmp(argv[1], "server") == 0) {
	startServer();
    }
    else if (strcmp(argv[1], "client") == 0) {
	startClient();
    }
    else {
	printf("first argument should be client/server\n");
    }
}
