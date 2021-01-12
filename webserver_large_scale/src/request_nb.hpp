#ifndef __REQUEST_NB_HPP_
#define __REQUEST_NB_HPP_
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

#define HDRDELIM       ": "
#define HDRDELIMSIZE   2
#define MAX_BODY_SIZE  
typedef enum Method { GET, POST } Method;

struct Http_request_nb {
    Method method;
    std::string target, version;
    std::unordered_map<std::string, std::string> header;

    struct bufferevent *bev;
    std::string body;
    char *body_buf;

    Http_request_nb(struct bufferevent *bev);
    ~Http_request_nb();
    bool recv_http_request(void);
    bool recv_http_request_header(void);
    bool recv_http_body(void);
    bool parse_startline(char *startline);
    bool parse_header(char *headerline);
    bool validate_header(void);
    int  get_body_length(void);
};

#endif