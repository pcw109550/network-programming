#ifndef __SERVER_HPP_
#define __SERVER_HPP_
#include "csapp.h"

struct Server {
    int clientlen, connfd;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    unsigned short client_port;

    Server(int listenfd);
    ~Server();
};

#endif