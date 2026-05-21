// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HTTP_HTTPRESPONSE_HPP_
#define SRC_HTTP_HTTPRESPONSE_HPP_

#include <map>
#include <string>

class HttpResponse {
 public:
  // Orthodox Canonical Form
  HttpResponse();
  HttpResponse(const HttpResponse& other);
  HttpResponse& operator=(const HttpResponse& other);
  ~HttpResponse();

  // Setters
  void set_status(int code, const std::string& phrase);
  void add_header(const std::string& key, const std::string& value);
  void set_body(const std::string& body);

  // Serialization
  std::string to_string() const;

 private:
  int _status_code;
  std::string _reason_phrase;
  std::map<std::string, std::string> _headers;
  std::string _body;
};

#endif  // SRC_HTTP_HTTPRESPONSE_HPP_
