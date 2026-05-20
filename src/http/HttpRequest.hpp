// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HTTP_HTTPREQUEST_HPP_
#define SRC_HTTP_HTTPREQUEST_HPP_

#include <map>
#include <string>

/**
 * @brief A passive data structure to store parsed HTTP elements.
 * 
 * This class stores the HTTP request method, URI, HTTP
 * version, body, and headers.
 * It provides public getters to access the parsed values
 * and public setters for the parser to populate them,
 * plus a clear() method to reset the state.
 */
class HttpRequest {
 public:
  HttpRequest();
  ~HttpRequest();

  // Getters returning const references
  const std::string& get_method() const;
  const std::string& get_uri() const;
  const std::string& get_version() const;
  const std::string& get_body() const;
  const std::map<std::string, std::string>& get_headers() const;

  // Setters/mutators for the parser
  void set_method(const std::string& method);
  void set_uri(const std::string& uri);
  void set_version(const std::string& version);
  void set_body(const std::string& body);
  void add_header(const std::string& key,
                  const std::string& value);

  // Reset method
  void clear();

 private:
  std::string                        _method;
  std::string                        _uri;
  std::string                        _version;
  std::string                        _body;
  std::map<std::string, std::string> _headers;
};

#endif  // SRC_HTTP_HTTPREQUEST_HPP_
