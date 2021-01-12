#include "csapp.h"
#include "server.hpp"

Server::Server(int listenfd) {
    clientlen = sizeof(clientaddr); 
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
    hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                    sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    haddrp = inet_ntoa(clientaddr.sin_addr);
    client_port = ntohs(clientaddr.sin_port);
#ifdef DEBUG
    std::cout << "server connected to " << hp->h_name \
              << " (" << haddrp << "), port " << client_port << std::endl;
#endif
}

Server::~Server() {
    Close(connfd);
}
