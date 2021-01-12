#ifndef __REQUEST_HPP_
#define __REQUEST_HPP_
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "csapp.h"

#define CRLF           "\r\n"
#define CRLFLEN        2
#define HDRDELIM       ": "
#define HDRDELIMSIZE   2
#define MAX_BODY_SIZE  
typedef enum Method { GET, POST } Method;

struct Http_request {
    Method method;
    std::string target, version;
    std::unordered_map<std::string, std::string> header;

    std::string body;
    char *buf, *body_buf;
    rio_t rio;

    Http_request(int connfd);
    ~Http_request();
    bool recv_http_request(void);
    bool recv_http_request_header(void);
    bool recv_http_body(void);
    bool parse_startline(void);
    bool parse_header(void);
    bool validate_header(void);
    int  get_body_length(void);
};

#endif