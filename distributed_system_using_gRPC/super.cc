#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <assert.h>
#include "sockserver.hpp"
#include "file.hpp"
#include "super.hpp"
#include "csapp.h"
#include "keyvalueserver.hpp"
#include "keyvalueclient.hpp"

bool is_main_super_node;
fetchaddr_state addr_state;
std::vector<std::string> child_addresses;

int main (int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " [recv_port] [gRPC port] [child_ip:child_port]" << std::endl;
        std::cerr << "Usage: " << argv[0] << " [recv_port] [gRPC port] -s [another super node's ip_address]:[another super node's port] [child_ip:child_port]s" << std::endl;
        return EXIT_FAILURE;
    }
    char *server_port = argv[1];
    std::string grpc_port(argv[2]);
    /* IMPORTANT: super node itself thinks its ip address is 127.0.0.1 
       In actual scenario, it must be specific ip or domain */

    std::string current_super_node_address = "127.0.0.1:" + grpc_port;

    for (int i = 3; i < argc; i++) {
        std::string temp(argv[i]);
        if (i == 3 && temp == "-s") {
            assert(argc >= 5);
            is_main_super_node = true;
            i++;
            addr_state.other_super_node_address = std::string(argv[4]);
            continue;
        }
        child_addresses.push_back(std::string(argv[i]));
    }
    show_status(server_port, grpc_port, addr_state.other_super_node_address, child_addresses);

    std::thread supernode_server_thread(supernode_server_task, grpc_port);
    KeyValueStoreClient *supernode_client;
    if (is_main_super_node) {
        // main supernode open connection
        supernode_client = new KeyValueStoreClient(addr_state.other_super_node_address);
        // main supernode sends its address to other supernode
        supernode_client->SendAddress(current_super_node_address);
    }
    if (!is_main_super_node) {
        // other supernode gets main supernode's address
        std::unique_lock<std::mutex> lck(addr_state.mtx);
        while (!addr_state.ready) {
            addr_state.cv.wait(lck);
        }
        std::cout << "Other supernode fetched main supernode address: " << addr_state.other_super_node_address << std::endl;
    }    
    if (!is_main_super_node) {
        // other supernode open connection
        supernode_client = new KeyValueStoreClient(addr_state.other_super_node_address);
    }

    // Process requests from client
    supernode_client->HandleClientConnection(server_port, child_addresses, addr_state.other_super_node_address);

    supernode_server_thread.join();
    delete supernode_client;

    return EXIT_SUCCESS;
}

void supernode_server_task(std::string grpc_port) {
    KeyValueStoreServer supernode_server(grpc_port, child_addresses, &addr_state, false);
}

void show_status(char *server_port, std::string &grpc_port,
        std::string &other_super_node_address,
        std::vector<std::string> &child_addresses) {
    std::cout << "###### SUPERNODE STATUS ######" << std::endl;
    std::cout << "[Server port]: " << server_port << std::endl;
    std::cout << "[gRPC port]:   " << grpc_port << std::endl;
    if (is_main_super_node) {
        std::cout << "[Main supernode]" << std::endl;
        std::cout << "[other supernode address]:" << other_super_node_address << std::endl;
    } else {
        std::cout << "[Sub supernode]" << std::endl;
    }
    std::cout << "## Total " << child_addresses.size() << " child node(s) ## " << std::endl;
    for (int i = 0; i < child_addresses.size(); i++)
        std::cout << "[Childnode #" << i << "]: " << child_addresses[i] << std::endl;
    std::cout << "##############################" << std::endl;
}