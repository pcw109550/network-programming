#include <iostream>
#include <string>
#include "csapp.h"
#include "client.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "redis.hpp"
#include "webserver_fork.hpp"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [webserver_port] [redis_server_ip] [redis_server_port]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int webserver_listenfd;
    char *webserver_port = argv[1];
    char *redis_server_ip = argv[2];
    char *redis_server_port = argv[3];

    Signal(SIGCHLD, sigchld_handler);

    webserver_listenfd = Open_listenfd(webserver_port); 

    while (true) {
        Server *server = new Server(webserver_listenfd);
        if (Fork() == 0) {
            close(webserver_listenfd);

            Client *client = new Client(redis_server_ip, redis_server_port);
            Http_request  *http_request = new Http_request(server->connfd);
            Http_response *http_response = new Http_response(server->connfd);
            bool valid = http_request->recv_http_request();
            if (!valid) {
                http_response->send_http_response_bad_request(http_request);
                delete http_response;
                delete http_request;
                delete server;
                delete client;

                exit(EXIT_SUCCESS);
            }
            Redis *redis = new Redis(client->clientfd);
            redis->send_redis_request(http_request);
            redis->recv_redis_response();
            http_response->send_http_response(http_request, redis);

            delete http_response;
            delete redis;
            delete http_request;
            delete server;
            delete client;

            exit(EXIT_SUCCESS);
        }
        delete server;
    }
    close(webserver_listenfd);

    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages
    // https://programmer.group/redis-serialization-protocol.html
    return EXIT_SUCCESS;
}

void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
#ifdef DEBUG
        fprintf(stderr, "server child exit %d sig %d\n", pid, sig);
#endif
    }
}