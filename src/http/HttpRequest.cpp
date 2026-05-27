// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpRequest.hpp"

#include <map>
#include <string>

/**
 * @brief Default constructor for HttpRequest.
 */
HttpRequest::HttpRequest() {}

/**
 * @brief Destructor for HttpRequest.
 */
HttpRequest::~HttpRequest() {}

/**
 * @brief Gets the HTTP method.
 * @return const std::string& The method (e.g., GET, POST).
 */
const std::string& HttpRequest::get_method() const {
  return _method;
}

/**
 * @brief Gets the requested URI.
 * @return const std::string& The URI.
 */
const std::string& HttpRequest::get_uri() const {
  return _uri;
}

/**
 * @brief Gets the HTTP version.
 * @return const std::string& The version (e.g., HTTP/1.1).
 */
const std::string& HttpRequest::get_version() const {
  return _version;
}

/**
 * @brief Gets the request body.
 * @return const std::string& The body content.
 */
const std::string& HttpRequest::get_body() const {
  return _body;
}

/**
 * @brief Gets all HTTP headers.
 * @return const std::map<std::string, std::string>& The headers map.
 */
const std::map<std::string, std::string>& HttpRequest::get_headers() const {
  return _headers;
}

/**
 * @brief Gets all parsed cookies.
 * @return const std::map<std::string, std::string>& The cookies map.
 */
const std::map<std::string, std::string>& HttpRequest::get_cookies() const {
  return _cookies;
}

/**
 * @brief Sets the HTTP method.
 * @param method The HTTP method string.
 */
void HttpRequest::set_method(const std::string& method) {
  _method = method;
}

/**
 * @brief Sets the HTTP URI.
 * @param uri The requested URI string.
 */
void HttpRequest::set_uri(const std::string& uri) {
  _uri = uri;
}

/**
 * @brief Sets the HTTP version.
 * @param version The HTTP version string.
 */
void HttpRequest::set_version(const std::string& version) {
  _version = version;
}

/**
 * @brief Sets the request body.
 * @param body The body string.
 */
void HttpRequest::set_body(const std::string& body) {
  _body = body;
}

/**
 * @brief Clears all request data, resetting the object to an empty state.
 */
void HttpRequest::clear() {
  _method.clear();
  _uri.clear();
  _version.clear();
  _body.clear();
  _headers.clear();
  _cookies.clear();
}
