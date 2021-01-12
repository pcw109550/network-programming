#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include "csapp.h"
#include "request.hpp"

// #define DEBUG

Http_request::Http_request(int connfd) {
    Rio_readinitb(&rio, connfd);
    buf = new char[MAXLINE];
}

Http_request::~Http_request() {
    delete[] buf;
}

bool Http_request::recv_http_request(void) {
    if (!recv_http_request_header())
        return false;
    if (!recv_http_body())
        return false;
    return true;
}

bool Http_request::recv_http_body(void) {
    int content_length = get_body_length();
    if (content_length == 0)
        return true;
    body_buf = new char[content_length + 1];
    std::fill(body_buf, body_buf + content_length + 1, '\0');
    int n = Rio_readnb(&rio, body_buf, content_length);
    body = body_buf;
    delete[] body_buf;
    if (n != content_length) {
        std::cerr << "Cannot receive http body" << std::endl;
        return false;
    }
#ifdef DEBUG
    std::cout << "body: " << body.size() << std::endl;
#endif
    return true;
}

int Http_request::  get_body_length(void) {
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

bool Http_request::recv_http_request_header(void) {
    size_t n;
    Rio_readcrlfb(&rio, buf, MAXLINE);
    if (!parse_startline())
        return false;
    while((n = Rio_readcrlfb(&rio, buf, MAXLINE)) != 0) { 
        std::string headerline (buf);
        if (CRLF == headerline)
            break; // need to validate header
        if (!parse_header())
            return false;
    }
    if (!validate_header())
        return false;
    return true;
}

bool Http_request::validate_header(void) {
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

bool Http_request::parse_header(void) {
    std::string headerline (buf);
    headerline = headerline.substr(0, headerline.size() - CRLFLEN);
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

bool Http_request::parse_startline(void) {
    std::string startline (buf), token;
    startline = startline.substr(0, startline.size() - CRLFLEN);
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