#ifndef __SUPER_HPP_
#define __SUPER_HPP_
#include <string>
#include <vector>
#include "keyvalueserver.hpp"

void show_status(char *server_port, std::string &grpc_port,
        std::string &other_super_node_address,
        std::vector<std::string> &child_addresses);

void supernode_server_task(std::string grpc_port);

#endif