#ifndef __SOCKSERVER_HPP_
#define __SOCKSERVER_HPP_
#include "csapp.h"

struct SockServer {
    int clientlen, connfd;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    unsigned short client_port;

    SockServer(int listenfd);
    ~SockServer();
};

#endif