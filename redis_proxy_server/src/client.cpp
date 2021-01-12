#include "client.hpp"

Client::Client(char *host, char *port) {
    clientfd = Open_clientfd(host, port);
}

Client::~Client() {
    Close(clientfd);
}