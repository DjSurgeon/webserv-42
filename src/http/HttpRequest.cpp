#include "http/HttpRequest.hpp"

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

void HttpRequest::add_header(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void HttpRequest::clear() {
    _method.clear();
    _uri.clear();
    _version.clear();
    _body.clear();
    _headers.clear();
}
