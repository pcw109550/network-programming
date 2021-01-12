#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include "csapp.h"
#include "keyvalueserver.hpp"
#include "keyvalueclient.hpp"
#include "child.hpp"

int main (int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [recv_port] [super node's ip_address]:[another super node's gRPC port] [DB server’s ip_address]:[DB server’s port]" << std::endl;
       return EXIT_FAILURE;
    }
    std::string server_port(argv[1]);
    std::string super_node_address(argv[2]);
    std::string db_server_address(argv[3]);

    show_status(server_port, super_node_address, db_server_address);
    std::vector<std::string> child_addresses;
    child_addresses.push_back(db_server_address);
    KeyValueStoreServer server(server_port, child_addresses, NULL, true);
    
    return EXIT_SUCCESS;
}

void show_status(std::string &server_port, std::string &super_node_address,
         std::string &db_server_address) {
    std::cout << "###### CHILDNODE STATUS ######" << std::endl;
    std::cout << "[Server port]: " << server_port << std::endl;
    std::cout << "[supernode address]: " << super_node_address << std::endl;
    std::cout << "[DB server address]: " << db_server_address << std::endl;
    std::cout << "##############################" << std::endl;
}