// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_HTTP_HTTPREQUEST_HPP_
#define INCLUDE_HTTP_HTTPREQUEST_HPP_

#include <map>
#include <string>
#include <vector>

struct LanguageWeight {
  std::string lang;
  double q;
};

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

  const std::string& get_method() const;
  const std::string& get_uri() const;
  const std::string& get_version() const;
  const std::string& get_body() const;
  const std::map<std::string, std::string>& get_headers() const;
  const std::map<std::string, std::string>& get_cookies() const;
  std::vector<LanguageWeight> get_accepted_languages() const;

  void set_method(const std::string& method);
  void set_uri(const std::string& uri);
  void set_version(const std::string& version);
  void set_body(const std::string& body);
  void swap_body(std::string& body);
  void add_header(const std::string& key, const std::string& value);
  void add_cookie(const std::string& key, const std::string& value);

  void reserve_body(size_t size);
  void clear();

 private:
  std::string _method;
  std::string _uri;
  std::string _version;
  std::string _body;
  std::map<std::string, std::string> _headers;
  std::map<std::string, std::string> _cookies;

  void _parse_cookies_string(const std::string& raw_cookies);
};

#endif  // INCLUDE_HTTP_HTTPREQUEST_HPP_
