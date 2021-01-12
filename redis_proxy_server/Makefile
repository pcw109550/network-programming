::Makefile for Assignment 2

# Update following lines as needed.

SERVER_FORK_SRC = src/webserver_fork.cpp
SERVER_FORK_HDR = src/webserver_fork.hpp

SERVER_THREAD_SRC = src/webserver_thread.cpp
SERVER_THREAD_HDR = src/webserver_thread.hpp

SERVER_LIBEVENT_SRC = src/webserver_libevent.cpp
SERVER_LIBEVENT_HDR = src/webserver_libevent.hpp

CSAPP_SRC = src/csapp.c
CSAPP_HDR = src/csapp.h

SERVER_SRC = src/server.cpp
SERVER_HDR = src/server.hpp

CLIENT_SRC = src/client.cpp
CLIENT_HDR = src/client.hpp

REQUEST_SRC = src/request.cpp
REQUEST_HDR = src/request.hpp

RESPONSE_SRC = src/response.cpp
RESPONSE_HDR = src/response.hpp

REDIS_SRC = src/redis.cpp
REDIS_HDR = src/redis.hpp

##############################

CC=g++
CFLAGS=-I. -g -Wall -Wextra -pedantic -std=c++17
LDFLAGS= -lpthread -levent -levent_core
LIBS_PATH= -L/usr/local/lib

# sudo apt-get install libevent-dev
build: bin/webserver_fork bin/webserver_thread bin/webserver_libevent

bin/webserver_fork: $(SERVER_FORK_SRC) $(SERVER_FORK_HDR) $(CSAPP_SRC) $(CSAPP_HDR) \
			$(REQUEST_SRC) $(REQUEST_HDR) \
			$(REDIS_SRC) $(REDIS_HDR) \
			$(SERVER_SRC) $(SERVER_HDR) \
			$(CLIENT_SRC) $(CLIENT_HDR) \
			$(RESPONSE_SRC) $(RESPONSE_HDR) \
			bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_FORK_SRC) $(CSAPP_SRC) \
			$(RESPONSE_SRC) $(SERVER_SRC) $(REQUEST_SRC) $(REDIS_SRC) $(CLIENT_SRC) \
			$(LIBS_PATH) $(LDFLAGS)

bin/webserver_thread: $(SERVER_THREAD_SRC) $(SERVER_THREAD_HDR) $(CSAPP_SRC) $(CSAPP_HDR) \
			$(REQUEST_SRC) $(REQUEST_HDR) \
			$(REDIS_SRC) $(REDIS_HDR) \
			$(SERVER_SRC) $(SERVER_HDR) \
			$(CLIENT_SRC) $(CLIENT_HDR) \
			$(RESPONSE_SRC) $(RESPONSE_HDR) \
			bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_THREAD_SRC) $(CSAPP_SRC) \
			$(RESPONSE_SRC) $(SERVER_SRC) $(REQUEST_SRC) $(REDIS_SRC) $(CLIENT_SRC) \
			$(LIBS_PATH) $(LDFLAGS)

bin/webserver_libevent: $(SERVER_LIBEVENT_SRC) $(SERVER_LIBEVENT_HDR) $(CSAPP_SRC) $(CSAPP_HDR) \
			$(REQUEST_SRC) $(REQUEST_HDR) \
			$(REDIS_SRC) $(REDIS_HDR) \
			$(SERVER_SRC) $(SERVER_HDR) \
			$(CLIENT_SRC) $(CLIENT_HDR) \
			$(RESPONSE_SRC) $(RESPONSE_HDR) \
			bin
	$(CC) $(CFLAGS) -o $@ $(SERVER_LIBEVENT_SRC) $(CSAPP_SRC) \
			$(RESPONSE_SRC) $(SERVER_SRC) $(REQUEST_SRC) $(REDIS_SRC) $(CLIENT_SRC) \
			$(LIBS_PATH) $(LDFLAGS)

.PHONY: clean build

bin:
	mkdir -p bin

clean:
	rm -rf bin
