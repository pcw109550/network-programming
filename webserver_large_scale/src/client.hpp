#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_
#include "csapp.h"

struct Client {
    int clientfd;

    Client(char *host, char *port);
    ~Client();
};

#endif