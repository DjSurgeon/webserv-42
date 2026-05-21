// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpResponse.hpp"

#include <map>
#include <sstream>
#include <string>

HttpResponse::HttpResponse() : _status_code(200), _reason_phrase("OK") {}

HttpResponse::HttpResponse(const HttpResponse& other)
    : _status_code(other._status_code),
      _reason_phrase(other._reason_phrase),
      _headers(other._headers),
      _body(other._body) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
  if (this != &other) {
    _status_code = other._status_code;
    _reason_phrase = other._reason_phrase;
    _headers = other._headers;
    _body = other._body;
  }
  return *this;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::set_status(int code, const std::string& phrase) {
  _status_code = code;
  _reason_phrase = phrase;
}

void HttpResponse::add_header(const std::string& key,
                              const std::string& value) {
  _headers[key] = value;
}

void HttpResponse::set_body(const std::string& body) {
  _body = body;
}

std::string HttpResponse::to_string() const {
  std::stringstream ss;

  ss << "HTTP/1.1 " << _status_code << " " << _reason_phrase << "\r\n";

  std::map<std::string, std::string>::const_iterator it;
  for (it = _headers.begin(); it != _headers.end(); ++it) {
    ss << it->first << ": " << it->second << "\r\n";
  }

  ss << "\r\n";
  ss << _body;

  return ss.str();
}
