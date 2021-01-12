#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <cctype>
#include "file.hpp"
#include "client.hpp"
#include "csapp.h"
#include "assert.h"

File::File(int _sockfd, char *_file_name) {
    sockfd = _sockfd;
    file_name = _file_name;
    file_content = recv_file_content = NULL;
    Rio_readinitb(&rio, sockfd);
}

File::~File() {
    if (file_name != NULL) {
        Close(file_fd);
        Close(recv_file_fd);
        delete []file_content;
    }
    delete []recv_file_content;
}

void  File::save_file(void) {
    std::string new_file_name(basename(file_name));
    new_file_name.append(DECRYPT_SUFFIX);
    recv_file_fd = Open(new_file_name.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0664);
    Write(recv_file_fd, recv_file_content, recv_file_size);
}

void  File::recv_file(void) {
    long n;
    long size = 0;
    n = Rio_readnb(&rio, reinterpret_cast<char *>(&size), HEADER_SIZE);
    assert(n == HEADER_SIZE);
    recv_file_size = ntohl(size);
    recv_file_content = new char[recv_file_size + 1];
    recv_file_content[recv_file_size] = '\0';
    n = Rio_readnb(&rio, recv_file_content, recv_file_size);
    assert(n == recv_file_size);
}

void  File::open_file(void) {
    get_filesize();
    file_fd = Open(file_name, O_RDONLY, 0);
}

void  File::echo_file(void) {
    // Just for debugging
    assert(file_name == NULL && recv_file_content != NULL);
    // send file size
    long size = htonl(recv_file_size);
    Rio_writen(sockfd, reinterpret_cast<char *>(&size), HEADER_SIZE);
    // send file content
    Rio_writen(sockfd, recv_file_content, recv_file_size);
}

void File::parse_file(void) {
    encrypted_file_content = std::string(recv_file_content);
    int start = 0, size = 0;
    for (int i = 0; i < recv_file_size; i++) {
        if (isalnum(static_cast<int>(encrypted_file_content[i]))) {
            size++;
        } else {
            if (size != 0) {
                std::string token = encrypted_file_content.substr(start, size);
                parse_result.insert(token);         
            }
            size = 0;
            start = i + 1;
        }
    }
}

void File::send_file(void) {
    file_content = new char[file_size + 1];
    file_content[file_size] = '\0';
    Read(file_fd, file_content, file_size);
    // send file size
    long size = htonl(file_size);
    Rio_writen(sockfd, reinterpret_cast<char *>(&size), HEADER_SIZE);
    // send file content
    Rio_writen(sockfd, file_content, file_size);
}

void File::decode(std::unordered_map<std::string, std::string> &map_result) {
    decrypted_file_content = std::string(recv_file_content);
    int start = 0, size = 0, decrypted_file_size = recv_file_size;
    int i = 0;
    while (i < decrypted_file_size) {
        if (isalnum(static_cast<int>(decrypted_file_content[i]))) {
            size++;
        } else {
            if (size != 0) {
                std::string key = decrypted_file_content.substr(start, size);
                std::string value = map_result[key];
                decrypted_file_content.replace(start, size, value);
                i += value.size() - size;
                decrypted_file_size += value.size() - size;
            }
            size = 0;
            start = i + 1;
        }
        i++;
    }
}

void File::send_decoded_file(void) {
    // send file content
    long size = htonl(decrypted_file_content.size());
    char *content = &decrypted_file_content[0];
    Rio_writen(sockfd, reinterpret_cast<char *>(&size), HEADER_SIZE);
    Rio_writen(sockfd, content, decrypted_file_content.size());
}

void File::get_filesize(void) {
    struct stat buffer;
    int status;
    status = stat(file_name, &buffer);
    if (status == 0 && S_ISREG(buffer.st_mode)) {
        file_size = buffer.st_size;
    } else {
        fprintf(stderr, "%s is not a proper txtfile\n", file_name);
        exit(EXIT_FAILURE);
    }
}