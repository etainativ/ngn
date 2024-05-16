#include <stdint.h>

typedef uint32_t msgsize_t;
typedef uint64_t token_t;

struct Client;
struct Server;

struct Datagram {
    msgsize_t size;
    void *data;
};

struct Client *clientInit(const char *hostname, uint16_t port, token_t token);
struct Server *serverInit(uint16_t port, int (*validator)(token_t token));

void clientSend(struct Client *client, struct Datagram data);
void serverSend(struct Server *server, struct Datagram data, token_t token);
void serverSendAll(struct Server *server, struct Datagram data);

void clientRecv(struct Client *client, struct Datagram *data);
void serverRecv(struct Server *server, struct Datagram *data, token_t *token);

void clientDestroy(struct Client *client);
void serverDestroy(struct Server *server);
