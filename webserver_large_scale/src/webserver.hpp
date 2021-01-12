#ifndef __WEBSERVER_LIBEVENT_HPP_
#define __WEBSERVER_LIBEVENT_HPP_
#include <event2/event.h>
#include <event2/util.h>
#include "server.hpp"
#include "queue.hpp"

#define THREAD_NUM        10
#define REDIS_CONN_NUM    40

Queue<Server **> task_queue;
Queue<Client **> client_queue;
struct event_base *Event_base_new(void);

int Set_nonblock(int fd);

void dummy_response(evutil_socket_t fd);
void thread_task(void);
void client_task(void);
void listener_callback(evutil_socket_t fd, short events, void *arg);

#endif