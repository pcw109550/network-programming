::Makefile for Assignment 3

# Update following lines as needed.
# You may change this file as you wish, as long as your Makefile can generate the target binary (webserver).

WEBSERVER_SRC = src/webserver.cpp
WEBSERVER_HDR = src/webserver.hpp

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

QUEUE_SRC = src/queue.cpp
QUEUE_HDR = src/queue.hpp

POOL_HDR = src/pool.hpp

##############################

CC=g++
CFLAGS=-I. -g -std=c++17 # -Wall -Wextra -pedantic
LDFLAGS= -lpthread -levent -levent_core
LIBS_PATH= -L/usr/local/lib

# sudo apt-get install libevent-dev
build: bin/webserver

bin/webserver: $(WEBSERVER_SRC) $(WEBSERVER_HDR) $(CSAPP_SRC) $(CSAPP_HDR) \
			$(REQUEST_SRC) $(REQUEST_HDR) \
			$(REDIS_SRC) $(REDIS_HDR) \
			$(SERVER_SRC) $(SERVER_HDR) \
			$(CLIENT_SRC) $(CLIENT_HDR) \
			$(RESPONSE_SRC) $(RESPONSE_HDR) \
			$(QUEUE_SRC) $(QUEUE_HDR) \
			$(POOL_HDR) \
			bin
	$(CC) $(CFLAGS) -o $@ $(WEBSERVER_SRC) $(CSAPP_SRC) \
			$(RESPONSE_SRC) $(SERVER_SRC) $(REQUEST_SRC) $(REDIS_SRC) $(CLIENT_SRC) \
			$(QUEUE_SRC) $(LIBS_PATH) $(LDFLAGS)

.PHONY: clean build

bin:
	mkdir -p bin

clean:
	rm -rf bin
