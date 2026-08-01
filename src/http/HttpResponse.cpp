// Copyright 2026 raperez- serjimen
#include "http/HttpResponse.hpp"

#include <map>
#include <sstream>
#include <string>
#include <vector>

/**
 * @brief Default constructor.
 * Initializes the response with a 200 OK status.
 */
HttpResponse::HttpResponse() : _status_code(200), _reason_phrase("OK") {}

/**
 * @brief Copy constructor.
 *
 * @param other The HttpResponse instance to copy from.
 */
HttpResponse::HttpResponse(const HttpResponse& other)
    : _status_code(other._status_code),
      _reason_phrase(other._reason_phrase),
      _headers(other._headers),
      _cookies(other._cookies),
      _body(other._body) {}

/**
 * @brief Copy assignment operator.
 *
 * @param other The HttpResponse instance to assign from.
 * @return A reference to this instance.
 */
HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
  if (this != &other) {
    _status_code = other._status_code;
    _reason_phrase = other._reason_phrase;
    _headers = other._headers;
    _cookies = other._cookies;
    _body = other._body;
  }
  return *this;
}

/**
 * @brief Destructor.
 */
HttpResponse::~HttpResponse() {}

/**
 * @brief Sets the HTTP status code and reason phrase.
 *
 * @param code The HTTP status code.
 * @param phrase The reason phrase.
 */
void HttpResponse::set_status(int code, const std::string& phrase) {
  _status_code = code;
  _reason_phrase = phrase;
}

/**
 * @brief Adds an HTTP header.
 *
 * @param key The header name.
 * @param value The header value.
 */
void HttpResponse::add_header(const std::string& key,
                              const std::string& value) {
  _headers[key] = value;
}

/**
 * @brief Adds a Set-Cookie header constructed from components.
 *
 * @param name The cookie name.
 * @param value The cookie value.
 * @param options Additional cookie options.
 */
void HttpResponse::add_cookie(const std::string& name, const std::string& value,
                              const std::string& options) {
  std::string cookie_str = name + "=" + value;
  if (!options.empty()) {
    cookie_str += "; " + options;
  }
  _cookies.push_back(cookie_str);
}

/**
 * @brief Adds a raw Set-Cookie string directly.
 *
 * @param raw_cookie The complete cookie string.
 */
void HttpResponse::add_cookie(const std::string& raw_cookie) {
  _cookies.push_back(raw_cookie);
}

/**
 * @brief Sets the body of the response.
 *
 * @param body The payload.
 */
void HttpResponse::set_body(const std::string& body) {
  _body = body;
}

/**
 * @brief Serializes the response into a raw HTTP string.
 *
 * @return The complete HTTP response as a string.
 */
std::string HttpResponse::to_string() const {
  std::stringstream ss;

  ss << "HTTP/1.1 " << _status_code << " " << _reason_phrase << "\r\n";

  std::map<std::string, std::string>::const_iterator it;
  for (it = _headers.begin(); it != _headers.end(); ++it) {
    ss << it->first << ": " << it->second << "\r\n";
  }

  for (size_t i = 0; i < _cookies.size(); ++i) {
    ss << "Set-Cookie: " << _cookies[i] << "\r\n";
  }

  ss << "\r\n";
  ss << _body;

  return ss.str();
}
