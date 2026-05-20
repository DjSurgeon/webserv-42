// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpRequest.hpp"

#include <cctype>
#include <map>
#include <string>

HttpRequest::HttpRequest() {}

HttpRequest::~HttpRequest() {}

const std::string& HttpRequest::get_method() const {
    return _method;
}

const std::string& HttpRequest::get_uri() const {
    return _uri;
}

const std::string& HttpRequest::get_version() const {
    return _version;
}

const std::string& HttpRequest::get_body() const {
    return _body;
}

const std::map<std::string, std::string>& HttpRequest::get_headers() const {
    return _headers;
}

void HttpRequest::set_method(const std::string& method) {
    _method = method;
}

void HttpRequest::set_uri(const std::string& uri) {
    _uri = uri;
}

void HttpRequest::set_version(const std::string& version) {
    _version = version;
}

void HttpRequest::set_body(const std::string& body) {
    _body = body;
}

static std::string to_lower(const std::string& str) {
    std::string lower = str;
    for (size_t i = 0; i < lower.length(); ++i) {
        lower[i] = std::tolower(static_cast<unsigned char>(lower[i]));
    }
    return lower;
}

void HttpRequest::add_header(const std::string& key, const std::string& value) {
    _headers[to_lower(key)] = value;
}

void HttpRequest::clear() {
    _method.clear();
    _uri.clear();
    _version.clear();
    _body.clear();
    _headers.clear();
}
