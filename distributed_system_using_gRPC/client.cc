#include <iostream>
#include "assert.h"
#include "client.hpp"
#include "file.hpp"
#include "csapp.h"

int main (int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [input_file] [supernode_ip] [supernode_port]" << std::endl;
        return EXIT_FAILURE;
    }
    char *file_name = argv[1];
    char *supernode_ip = argv[2];
    char *supernode_port = argv[3];
    
    Client *client = new Client(supernode_ip, supernode_port);
    File *file = new File(client->clientfd, file_name);

    file->open_file();
    file->send_file();
    file->recv_file();
    file->save_file();

    delete client;
    delete file;

    return EXIT_SUCCESS;
}

Client::Client(char *host, char *port) {
    clientfd = Open_clientfd(host, port);
}

Client::~Client() {
    Close(clientfd);
}