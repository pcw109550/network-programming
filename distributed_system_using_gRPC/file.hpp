#ifndef __FILE_HPP_
#define __FILE_HPP_
#include "csapp.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>

#define DECRYPT_SUFFIX ".decoded"

struct File {
    char *file_name;
    long file_size, recv_file_size;
    rio_t rio;
    int sockfd, file_fd, recv_file_fd;
    char *file_content, *recv_file_content;
    std::string encrypted_file_content, decrypted_file_content;
    std::unordered_set<std::string> parse_result;

    File(int _sockfd, char *_file_name);
    ~File();

    void  get_filesize(void);
    void  open_file(void);
    void  send_file(void);
    void  recv_file(void);
    void  save_file(void);
    void  echo_file(void);
    void  parse_file(void);
    void  decode(std::unordered_map<std::string, std::string> &map_result);
    void  send_decoded_file(void);
};

#endif