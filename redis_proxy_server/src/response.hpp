#ifndef __RESPONSE_HPP_
#define __RESPONSE_HPP_
#include <iostream>
#include <string>
#include <sstream>
#include "csapp.h"
#include "request.hpp"
#include "redis.hpp"

#define CRLF           "\r\n"
#define CRLFLEN        2
#define HDRDELIM       ": "
#define HDRDELIMSIZE   2
typedef enum Status_code { OK = 200, BAD_REQUEST = 400, NOT_FOUND = 404 } Status_code;

struct Http_response {
    std::string version, status_text;
    Status_code status_code;
    std::string header, status_line, body, response;

    int connfd;
    rio_t rio;

    Http_response(int fd);
    ~Http_response();
    void form_status_line(Http_request *http_request, Redis *redis);
    void form_header(Redis *redis);
    void form_body(Redis *redis);
    void send_http_response(Http_request *http_request, Redis *redis);
    void send_http_response_bad_request(Http_request *http_request);
};

#endif