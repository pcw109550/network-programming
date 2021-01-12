# Makefile for Assignment 1

# Update following lines as needed.
# You may change this file as you wish, as long as your Makefile can generate three binaries (server, client, and client_multi).

COMMON_HEADER = src/common.h
COMMON_SOURCE = src/common.c

CLIENT_HEADER = src/client.h
CLIENT_SOURCE = src/client.c

CLIENT_MULTI_HEADER = src/client_multi.h
CLIENT_MULTI_SOURCE = src/client_multi.c

SERVER_HEADER = src/server.h
SERVER_SOURCE = src/server.c

CSAPP_HEADER = src/csapp.h
CSAPP_SOURCE = src/csapp.c

##############################

CC=gcc
CFLAGS=-I. -g -pthread -Wall
 
build: client client_multi server

client: $(CLIENT_SOURCE) $(CLIENT_HEADER) $(CSAPP_HEADER) $(COMMON_SOURCE) bin
	$(CC) $(CFLAGS) -o bin/$@ $(CLIENT_SOURCE) $(CSAPP_SOURCE) $(COMMON_SOURCE)

client_multi: $(CLIENT_MULTI_SOURCE) $(CLIENT_MULTI_HEADER) $(CSAPP_HEADER) $(COMMON_SOURCE) bin
	$(CC) $(CFLAGS) -o bin/$@ $(CLIENT_MULTI_SOURCE) $(CSAPP_SOURCE) $(COMMON_SOURCE)

server: $(SERVER_SOURCE) $(SERVER_HEADER) $(CSAPP_HEADER) $(COMMON_SOURCE) bin
	$(CC) $(CFLAGS) -o bin/$@ $(SERVER_SOURCE) $(CSAPP_SOURCE) $(COMMON_SOURCE)
    
.PHONY: clean test build

bin:
	mkdir -p bin

test: client server
	test/simple.sh

clean:
	rm -rf bin
