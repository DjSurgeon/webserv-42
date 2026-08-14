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

bool HttpResponse::has_header(const std::string& key) const {
  return _headers.find(key) != _headers.end();
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
 * @brief Sets the body directly from a substring to avoid extra memory copies.
 *
 * @param source The original string containing the body.
 * @param pos The starting position of the body in the source.
 */
void HttpResponse::set_body_from_substr(const std::string& source, size_t pos) {
  _body.assign(source, pos, std::string::npos);
}

/**
 * @brief Serializes the response into a raw HTTP string.
 *
 * @return The complete HTTP response as a string.
 */
void HttpResponse::to_string(std::string& raw_response) const {
  // 2. Estimación previa del tamaño para una única asignación de memoria
  size_t estimated_size = 30 + _reason_phrase.capacity() + _body.size();

  for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    estimated_size += it->first.size() + it->second.size() +
                      4;  // key + ": " + value + "\r\n"
  }
  for (size_t i = 0; i < _cookies.size(); ++i) {
    estimated_size +=
        14 + _cookies[i].size();  // "Set-Cookie: " + cookie + "\r\n"
  }

  raw_response.reserve(estimated_size);

  std::stringstream ss;
  ss << _status_code;

  raw_response.append("HTTP/1.1 ");
  raw_response.append(ss.str());
  ss.clear();
  raw_response.append(" ");
  raw_response.append(_reason_phrase);
  raw_response.append("\r\n");

  for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    raw_response.append(it->first);
    raw_response.append(": ");
    raw_response.append(it->second);
    raw_response.append("\r\n");
  }

  // Copia de cookies
  for (size_t i = 0; i < _cookies.size(); ++i) {
    raw_response.append("Set-Cookie: ");
    raw_response.append(_cookies[i]);
    raw_response.append("\r\n");
  }

  raw_response.append("\r\n");
  raw_response.append(_body);
}