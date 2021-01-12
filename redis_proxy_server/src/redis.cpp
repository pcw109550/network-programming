#include <iostream>
#include <string>
#include <sstream>
#include "csapp.h"
#include "redis.hpp"
#include "request.hpp"

// #define DEBUG

Redis::Redis(int clientfd) {
    connfd = clientfd;
    Rio_readinitb(&rio, connfd);
    buf = new char[MAXLINE];
}

Redis::~Redis() {
    delete[] buf;
}

int Redis::parse_startline(void) {
    int length = 0;
    std::string startline (buf);
    startline = startline.substr(0, startline.size() - CRLFLEN);
    if (startline.at(0) == '$') {
        try {
            length = std::stoi(startline.substr(1, startline.size()));
            if (length == -1)
                response = "ERROR";
            return length;
        } catch (int err) {
            std::cerr << "Invalid redis response" << std::endl;
            return -1;
        }
    }
    if (startline.compare("+OK"))
        return false;     // Assume never occur
    return length;
}

bool Redis::recv_redis_response_sets(void) {
    if (num_pairs < 1)
        return false;
    for (int i = 0; i < num_pairs - 1; i++) {
        Rio_readcrlfb(&rio, buf, MAXLINE);
        std::string response_line (buf);
        response_line = response_line.substr(0, response_line.size() - CRLFLEN);
        if (response_line.compare("+OK"))
            return false; // Assume never occur: every set command must return +OK
    }
#ifdef DEBUG
    std::cout << "SET: received OK"  << std::endl;
#endif
    response = "OK";
    return true;
}

bool Redis::recv_redis_response_get(void) {
    response_buf = new char[response_length + 1];
    std::fill(response_buf, response_buf + response_length + 1, '\0');
    int n = Rio_readnb(&rio, response_buf, response_length);
    if (n != response_length) {
        std::cerr << "Cannot receive value" << std::endl;
        return false;
    }
    response = response_buf;
    delete[] response_buf;
#ifdef DEBUG
    std::cout << "GET length: " << response.size() << std::endl;
#endif
    return true;
}

bool Redis::recv_redis_response(void) {
    is_set = false;
    is_get = false;
    Rio_readcrlfb(&rio, buf, MAXLINE);
    response_length = parse_startline();
    if (response_length == -1) {    // GET: non existing key
        return false;
    }
    if (response_length == 0) {     // SET: receive multiple +OK
        is_set = true;
        return recv_redis_response_sets();
    }
    if (response_length > 0) {      // GET: existing key
        is_get = true;
        return recv_redis_response_get();
    }
    return false;
}

void Redis::check_url_encoding(Http_request *http_request) {
    is_url_encoded = false;
    if (http_request->header.find("content-type") == http_request->header.end())
        return;
    std::string content_type = http_request->header["content-type"];
    if (!content_type.compare("application/x-www-form-urlencoded"))
        is_url_encoded = true;
}

void Redis::send_redis_request(Http_request *http_request) {
    if (http_request->method == GET) { // get
        is_url_encoded = true; // assume get target is always url encoded
        parse_http_target(http_request->target);
        form_redis_request_get();
    } else if (http_request->method == POST) { // set
        check_url_encoding(http_request);
        parse_http_body(http_request->body);
        form_redis_request_sets();
    }
    Rio_writen(connfd, (char *)(request.c_str()), request.size());
}

bool Redis::parse_http_body(std::string body) {
    std::string token, key, value;
    std::stringstream ss(body);
    while (std::getline(ss, token, '&')) {
        auto probe = token.find('=');
        if (probe == std::string::npos) {
            std::cerr << "Invalid body: " << body << std::endl;
            return false;
        }
        key = url_decode_if_needed(token.substr(0, probe));
        // value can be zero length string
        value = url_decode_if_needed(token.substr(probe + 1));
        if (key.size() == 0) {
            std::cerr << "Invalid key: " << body << std::endl;
            return false;
        }
        sets.push_back(std::make_pair(key, value));
#ifdef DEBUG
        std::cout << key.size() <<  ' ' << value.size() << std::endl;
#endif
    }
    num_pairs = sets.size();
    return true;
}

std::string Redis::url_decode(std::string &str) {
    std::string decoded;
    int length = str.length();
    char ch, hi, low;
    for (int i = 0; i < length; i++) {
        if (str[i] == '%' && i < length - 2 && isxdigit(str[i + 1]) && isxdigit(str[i + 2])) {
            hi = str[i + 1];
            low = str[i + 2];
            hi = hi <= '9' ? hi - '0' : hi <= 'Z' ? hi - 'A' + 10 : hi - 'a' + 10;
            low = low <= '9' ? low - '0' : low <= 'Z' ? low - 'A' + 10 : low - 'a' + 10;
            ch = (hi << 4) + low;
            decoded += ch;
            i += 2;
        } else if (str[i] == '+')
            decoded += ' ';
        else
            decoded += str[i];
    }
    return decoded;
}

std::string Redis::url_decode_if_needed(std::string str) {
    if (!is_url_encoded)
        return str;
    try {
        return url_decode(str);
    } catch(int expn) {
        std::cerr << "Url decode failure" << std::endl;
    }
    return str;
}

std::string Redis::form_redis_request_set(std::string key, std::string value) {
    std::stringstream ss;
    ss << "*3" << CRLF;
    ss << "$3" << CRLF << "SET" << CRLF;
    ss << "$" << key.size() << CRLF << key << CRLF;
    ss << "$" << value.size() << CRLF << value << CRLF;
    return ss.str();
}

void Redis::form_redis_request_sets(void) {
    std::stringstream ss;
    for (auto &set: sets) {
        ss << form_redis_request_set(set.first, set.second);
    }
    request = ss.str();
    sets.clear(); // not needed since used for formulation
}

bool Redis::parse_http_target(std::string target) {
    num_pairs = 0;
    if (target.at(0) != '/') {
        std::cerr << "Invalid header target" << std::endl;
        return false;
    }
    get_key = url_decode_if_needed(target.substr(1, target.size()));
#ifdef DEBUG
        std::cout << get_key.size() << std::endl;
#endif
    return true;
}

bool Redis::form_redis_request_get(void) {
    std::stringstream ss;
    ss << "*2" << CRLF;
    ss << "$3" << CRLF << "GET" << CRLF;
    ss << "$" << get_key.size() << CRLF << get_key << CRLF;
    request = ss.str();
    return true;
}