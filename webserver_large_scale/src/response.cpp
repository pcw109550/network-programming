#include <string>
#include <sstream>
#include "csapp.h"
#include "request.hpp"
#include "redis.hpp"
#include "response.hpp"

// #define DEBUG

Http_response::Http_response(int fd) {
    connfd = fd;
}

Http_response::~Http_response() {
}

void Http_response::send_http_response_bad_request(Http_request *http_request) {
    status_code = BAD_REQUEST;
    status_text = "Bad Request";
    version = http_request->version;
    status_line = version + " " + std::to_string(status_code) + " " + status_text + CRLF;
    std::stringstream ss;
    std::string content_type = "text/plain";
    ss << "content-type" << HDRDELIM << content_type << CRLF << CRLF;
    header = ss.str();
    response = status_line + header;
    Rio_writen(connfd, (char *)(response.c_str()), response.size());
}

void Http_response::send_http_response(Http_request *http_request, Redis *redis) {
    form_status_line(http_request, redis);
    form_header(redis);
    form_body(redis);
    response = status_line + header + body;
    Rio_writen(connfd, (char *)(response.c_str()), response.size());
}

void Http_response::form_body(Redis *redis) {
    body = redis->response;
}

void Http_response::form_header(Redis *redis) {
    std::stringstream ss;
    std::string content_type = status_code == NOT_FOUND ? "text/html" : "text/plain";
    int content_length;
    if (status_code == NOT_FOUND)
        content_length = 5; // strlen("ERROR");
    else if (redis->is_set)
        content_length = 2; // strlen("OK");
    else
        content_length = redis->response_length;
    ss << "content-type" << HDRDELIM << content_type << CRLF;
    ss << "content-length" << HDRDELIM << content_length << CRLF << CRLF;
    header = ss.str();
}

void Http_response::form_status_line(Http_request *http_request, Redis *redis) {
    status_code = (redis->is_get || redis->is_set) ? OK : NOT_FOUND;
    status_text = status_code == OK ? "OK" : "Not Found";
    version = http_request->version;
    status_line = version + " " + std::to_string(status_code) + " " + status_text + CRLF;
#ifdef DEBUG
    std::cout << status_line;
#endif  
}