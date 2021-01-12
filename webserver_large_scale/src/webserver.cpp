#include <iostream>
#include <chrono>
#include "csapp.h"
#include "client.hpp"
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "redis.hpp"
#include "webserver.hpp"
#include "queue.hpp"
#include "pool.hpp"
#include <event2/event.h>

// #define DEBUG

char *redis_server_ip;
char *redis_server_port;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [webserver_port] [redis_server_ip] [redis_server_port]\n", argv[0]);
        return EXIT_FAILURE;
    }
    evutil_socket_t webserver_listenfd;
    char *webserver_port = argv[1];
    redis_server_ip = argv[2];
    redis_server_port = argv[3];

    // Init thread pool
    Pool pool(THREAD_NUM);
    pool.enqueue(client_task);
    for (int i = 1; i < THREAD_NUM; i++) {
        pool.enqueue(thread_task);
    }

    // In case client fails
    Signal(SIGPIPE, SIG_IGN);

    struct event_base *base = Event_base_new();
    struct event *webserver_listener;
    
    webserver_listenfd = Open_listenfd(webserver_port);
    evutil_make_socket_nonblocking(webserver_listenfd);

    webserver_listener = event_new(base, webserver_listenfd, EV_READ|EV_WRITE|EV_PERSIST, listener_callback, NULL);
    // No timeout
    event_add(webserver_listener, NULL);

    event_base_dispatch(base);

    event_del(webserver_listener);
    event_free(webserver_listener);
    event_base_free(base);

    Close(webserver_listenfd);

    return EXIT_SUCCESS;
}

void dummy_response(evutil_socket_t fd) {
    std::string response ("HTTP/1.0 404 Not Found\r\ncontent-type: text/html\r\ncontent-length: 6\r\n\r\nERRORR");
    write(fd, response.c_str(), response.size());
}

void client_task(void) {
    while(true) {
        if (client_queue.size() >= REDIS_CONN_NUM)
            continue;
        Client **client_ptr = new Client*;
        *client_ptr = new Client(redis_server_ip, redis_server_port);
        client_queue.push(client_ptr);
    } 
}

void thread_task(void) {
    while (true) {
        Server **server_ptr = static_cast<Server **>(task_queue.pop());
        Server *server = *server_ptr;

        Http_request  *http_request = new Http_request(server->connfd);
        Http_response *http_response = new Http_response(server->connfd);
        bool valid = http_request->recv_http_request();
        if (!valid) {
            http_response->send_http_response_bad_request(http_request);
            delete http_response;
            delete http_request;
            delete server;
            delete server_ptr;
            continue;
        }
        Client **client_ptr = static_cast<Client **>(client_queue.pop());
        Client *client = *client_ptr;
        Redis *redis = new Redis(client->clientfd);
        redis->send_redis_request(http_request);
        redis->recv_redis_response();

        http_response->send_http_response(http_request, redis);

        delete http_response;
        delete redis;
        delete http_request;
        delete server;
        delete server_ptr;
        delete client;
        delete client_ptr;
    }
}

void listener_callback(evutil_socket_t fd, short events, void *arg) {
    Server **server_ptr = new Server*;
    evutil_socket_t webserver_listenfd = fd;
    *server_ptr = new Server(webserver_listenfd);
    task_queue.push(server_ptr);
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