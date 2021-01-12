#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "request_nb.hpp"

// #define DEBUG

Http_request_nb::Http_request_nb(struct bufferevent *_bev) {
    bev = _bev;
}

Http_request_nb::~Http_request_nb() {
}

bool Http_request_nb::recv_http_request(void) {
    if (!recv_http_request_header())
        return false;
    if (!recv_http_body())
        return false;
    return true;
}

bool Http_request_nb::recv_http_body(void) {
    int content_length = get_body_length();
    if (content_length == 0)
        return true;
    body_buf = new char[content_length + 1];
    std::fill(body_buf, body_buf + content_length + 1, '\0');
    bufferevent_read(bev, body_buf, content_length);
    body = body_buf;
    delete[] body_buf;
#ifdef DEBUG
    std::cout << "body: " << body.size() << std::endl;
#endif
    return true;
}

int Http_request_nb::  get_body_length(void) {
    int content_length = 0;
    if (header.find("content-length") == header.end())
        return content_length; // empty body
    try {
        content_length = std::stoi(header["content-length"]);
    } catch (int err) {
        std::cerr << "Invalid content length: " << header["content-length"] << std::endl;
        return content_length;
    }
    return content_length;
}

bool Http_request_nb::recv_http_request_header(void) {
    struct evbuffer *input = bufferevent_get_input(bev); size_t n = 0;
    char *startline = evbuffer_readln(input, &n, EVBUFFER_EOL_CRLF);
    if (!parse_startline(startline)) {
        free(startline);
        return false;
    }
    free(startline);
    while (true) {
        char *headerline = evbuffer_readln(input, &n, EVBUFFER_EOL_CRLF);
        if (n == 0) {
            free(headerline);
            break; // need to validate header
        }
        if (!parse_header(headerline)) {
            free(headerline);
            return false;
        }
        free(headerline);
    }
    if (!validate_header())
        return false;
    return true;
}

bool Http_request_nb::validate_header(void) {
    // RFC 7230: Http request header must have a host header field
    if (header.find("host") == header.end()) {
        std::cerr << "Invalid header: no host field" << std::endl;
        return false;
    }
    // https://piazza.com/class/kehxl6d0g1c3wu?cid=134
    // No content-length/content-type in post request
    if (method == POST) {
        if (header.find("content-length") == header.end()) {
            std::cerr << "Invalid header: no content-length field" << std::endl;
            return false;
        }
        if (header.find("content-type") == header.end()) {
            std::cerr << "Invalid header: no content-type field" << std::endl;
            return false;
        }
    }
    return true;
}

bool Http_request_nb::parse_header(char *_headerline) {
    std::string headerline (_headerline);
    auto probe = headerline.find(HDRDELIM);
    if (probe == std::string::npos) {
        std::cerr << "Invalid header: " << headerline << std::endl;
        return false;
    }
    std::string key = headerline.substr(0, probe);
    // HTTP header names are case-insensitive
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    std::string value = headerline.substr(probe + HDRDELIMSIZE);
    header[key] = value;
#ifdef DEBUG
    std::cout << key <<  ' ' << value << std::endl;
#endif
    return true;
}

bool Http_request_nb::parse_startline(char *_startline) {
    std::string startline (_startline), token;
    std::stringstream ss(startline);
    std::getline(ss, token, ' ');
    if (token != "GET" && token != "POST") { 
#ifdef DEBUG
        std::cerr << "Method not supported: " << token << std::endl;
#endif
        return false;
    }
    method = token == "GET" ? GET : POST;
    std::getline(ss, target, ' ');
    std::getline(ss, version, ' ');
    if (version.compare(0, 5, "HTTP/")) {
        std::cerr << "Invalid HTTP version: " << version << std::endl;
        return false;
    }
#ifdef DEBUG
    std::cout << token <<  ' ' << target.size() << ' ' << version << std::endl;
#endif
    return true;
}