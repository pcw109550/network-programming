#ifndef __REDIS_HPP_
#define __REDIS_HPP_
#include <iostream>
#include <string>
#include <vector>
#include "csapp.h"
#include "request.hpp"

struct Redis {
    rio_t rio;
    int connfd;
    // Use vector for ordering
    std::vector<std::pair<std::string, std::string> > sets;
    int num_pairs;
    std::string get_key;
    int response_length;
    std::string request, response;
    char *buf, *response_buf;
    bool is_set, is_get;
    bool is_url_encoded;

    Redis(int clientfd);
    ~Redis();
    void send_redis_request(Http_request *http_request);
    std::string form_redis_request_set(std::string key, std::string value);
    void form_redis_request_sets(void);
    bool form_redis_request_get(void);
    bool parse_http_target(std::string target);
    bool parse_http_body(std::string body);
    bool recv_redis_response(void);
    bool recv_redis_response_sets(void);
    bool recv_redis_response_get(void);
    int  parse_startline(void);
    void check_url_encoding(Http_request *http_request);
    std::string url_decode_if_needed(std::string str);
    std::string url_decode(std::string &str);
};

#endif