// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_HTTP_HTTPRESPONSE_HPP_
#define INCLUDE_HTTP_HTTPRESPONSE_HPP_

#include <map>
#include <string>
#include <vector>

class Context;
class HttpRequest;

/**
 * @brief Represents an HTTP response.
 *
 * This class encapsulates the status code, headers, cookies, and body
 * of an HTTP response. It provides methods to construct the response
 * and serialize it into a raw string suitable for transmission over a socket.
 */
class HttpResponse {
 public:
  HttpResponse();
  HttpResponse(const HttpResponse& other);
  HttpResponse& operator=(const HttpResponse& other);
  ~HttpResponse();

  void set_status(int code, const std::string& phrase);
  void add_header(const std::string& key, const std::string& value);
  void add_cookie(const std::string& name, const std::string& value,
                  const std::string& options = "");
  void add_cookie(const std::string& raw_cookie);
  void set_body(const std::string& body);

  std::string to_string() const;

  void generate_error_response(int code, const Context* ctx = NULL,
                               const HttpRequest* req = NULL);

 private:
  int _status_code;
  std::string _reason_phrase;
  std::map<std::string, std::string> _headers;
  std::vector<std::string> _cookies;
  std::string _body;

  std::string _extract_username(const HttpRequest* req) const;
  bool _try_serve_custom_error_page(int code, const Context* ctx);
  void _finalize_html_response(const std::string& body);

  std::string _get_default_error_html(int code, const std::string& phrase,
                                      const std::string& username) const;
};

#endif  // INCLUDE_HTTP_HTTPRESPONSE_HPP_
