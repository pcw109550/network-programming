#include <iostream>
#include "csapp.h"
#include "client.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "redis.hpp"
#include "webserver_thread.hpp"

char *redis_server_ip;
char *redis_server_port;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [webserver_port] [redis_server_ip] [redis_server_port]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int webserver_listenfd;
    char *webserver_port = argv[1];
    redis_server_ip = argv[2];
    redis_server_port = argv[3];
    
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    webserver_listenfd = Open_listenfd(webserver_port);

    while (true) {
        Server **server_ptr = new Server*;
        *server_ptr = new Server(webserver_listenfd);
        Pthread_create(&tid, &attr, &task, server_ptr);
    }
    pthread_attr_destroy(&attr);

    Close(webserver_listenfd);

    return EXIT_SUCCESS;
}

static void *task(void *arg) {
    Server *server = *static_cast<Server **>(arg);
    delete static_cast<Server **>(arg);

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

        return EXIT_SUCCESS;
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

    return EXIT_SUCCESS;
}