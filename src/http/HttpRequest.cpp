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

const std::map<std::string, std::string>& HttpRequest::get_cookies() const {
  return _cookies;
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

static std::string trim_spaces_cookie(const std::string& str) {
  size_t start = 0;
  while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  size_t end = str.length();
  while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t')) {
    end--;
  }
  return str.substr(start, end - start);
}

void HttpRequest::_parse_cookies_string(const std::string& raw_cookies) {
  size_t start = 0;
  while (start < raw_cookies.length()) {
    size_t end = raw_cookies.find(';', start);
    if (end == std::string::npos) {
      end = raw_cookies.length();
    }
    std::string pair = raw_cookies.substr(start, end - start);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      std::string key = trim_spaces_cookie(pair.substr(0, eq_pos));
      std::string value = trim_spaces_cookie(pair.substr(eq_pos + 1));
      if (!key.empty()) {
        _cookies[key] = value;
      }
    }
    start = end + 1;
  }
}

void HttpRequest::add_header(const std::string& key, const std::string& value) {
  std::string lower_key = to_lower(key);
  _headers[lower_key] = value;
  if (lower_key == "cookie") {
    _parse_cookies_string(value);
  }
}

void HttpRequest::clear() {
  _method.clear();
  _uri.clear();
  _version.clear();
  _body.clear();
  _headers.clear();
  _cookies.clear();
}
