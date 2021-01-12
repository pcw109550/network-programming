#include <iostream>
#include "csapp.h"
#include "client.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "redis.hpp"
#include "webserver_libevent.hpp"
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

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

    // In case client fails
    Signal(SIGPIPE, SIG_IGN);

    struct event_base *base = Event_base_new();
    struct event *webserver_listener;
    
    webserver_listenfd = Open_listenfd(webserver_port);
    Set_nonblock(webserver_listenfd);

    // No args for callback function
    // I can boost performance by seperating READ | WRITE event
    webserver_listener = event_new(base, webserver_listenfd, EV_READ | EV_WRITE | EV_PERSIST, task_callback, NULL);
    // No timeout
    event_add(webserver_listener, NULL);

    event_base_dispatch(base);

    event_del(webserver_listener);
    event_free(webserver_listener);
    event_base_free(base);

    Close(webserver_listenfd);

    return EXIT_SUCCESS;
}

void task_callback(evutil_socket_t fd, short events, void *arg) {
    // huge callback function which READ/WRITES
    int webserver_listenfd = fd;

    Server *server = new Server(webserver_listenfd);
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

        return;
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

    return;
}

struct event_base *Event_base_new(void) {
    struct event_base *base;
    if (!(base = event_base_new())) {
        std::cerr << "event_base_new error" << std::endl;
        exit(EXIT_FAILURE);
    }
    return base;
}

int Set_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
        std::cerr << "Set_nonblock error" << std::endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}