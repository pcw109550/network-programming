#ifndef __WEBSERVER_LIBEVENT_HPP_
#define __WEBSERVER_LIBEVENT_HPP_
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

struct event_base *Event_base_new(void);

void task_callback(evutil_socket_t fd, short events, void *arg);
int Set_nonblock(int fd);

#endif