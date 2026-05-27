// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpResponse.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "config/Context.hpp"
#include "http/HttpRequest.hpp"
#include "http/SessionManager.hpp"

static std::string get_reason_phrase(int code) {
  switch (code) {
    case 400:
      return "Bad Request";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    case 500:
      return "Internal Server Error";
    case 502:
      return "Bad Gateway";
    default:
      return "Internal Server Error";
  }
}

HttpResponse::HttpResponse() : _status_code(200), _reason_phrase("OK") {}

HttpResponse::HttpResponse(const HttpResponse& other)
    : _status_code(other._status_code),
      _reason_phrase(other._reason_phrase),
      _headers(other._headers),
      _cookies(other._cookies),
      _body(other._body) {}

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

HttpResponse::~HttpResponse() {}

void HttpResponse::set_status(int code, const std::string& phrase) {
  _status_code = code;
  _reason_phrase = phrase;
}

void HttpResponse::add_header(const std::string& key,
                              const std::string& value) {
  _headers[key] = value;
}

void HttpResponse::add_cookie(const std::string& name, const std::string& value,
                              const std::string& options) {
  std::string cookie_str = name + "=" + value;
  if (!options.empty()) {
    cookie_str += "; " + options;
  }
  _cookies.push_back(cookie_str);
}

void HttpResponse::add_cookie(const std::string& raw_cookie) {
  _cookies.push_back(raw_cookie);
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

  for (size_t i = 0; i < _cookies.size(); ++i) {
    ss << "Set-Cookie: " << _cookies[i] << "\r\n";
  }

  ss << "\r\n";
  ss << _body;

  return ss.str();
}

std::string HttpResponse::_get_default_error_html(
    int code, const std::string& phrase, const std::string& username) const {
  std::stringstream ss;
  ss << "<html><body><h1>" << code << " " << phrase << "</h1>";
  if (!username.empty()) {
    ss << "<p>Lo sentimos, <b>" << username << "</b>, este recurso no existe en el servidor.</p>";
  }
  ss << "</body></html>";
  return ss.str();
}

void HttpResponse::generate_error_response(int code, const Context* ctx,
                                           const HttpRequest* req) {
  std::string phrase = get_reason_phrase(code);

  if (phrase == "Internal Server Error" && code != 500) {
    code = 500;
  }

  set_status(code, phrase);

  std::string username = "";
  if (req) {
    const std::map<std::string, std::string>& cookies = req->get_cookies();
    std::map<std::string, std::string>::const_iterator it = cookies.find("session_id");
    if (it != cookies.end()) {
      SessionData* sdata = SessionManager::get_instance().get_session(it->second);
      if (sdata) {
        username = sdata->username;
      }
    }
  }

  if (ctx) {
    const std::map<int, std::string>& err_pages = ctx->get_error_pages();
    std::map<int, std::string>::const_iterator it = err_pages.find(code);
    if (it != err_pages.end()) {
      std::string err_uri = it->second;
      std::string root = ctx->get_root();
      if (!root.empty() && root[root.length() - 1] == '/') {
        root = root.substr(0, root.length() - 1);
      }
      if (!err_uri.empty() && err_uri[0] != '/') {
        err_uri = "/" + err_uri;
      }
      std::string physical_path = root + err_uri;

      std::ifstream file(physical_path.c_str(), std::ios::binary);
      if (file.is_open()) {
        std::ostringstream buffer;
        buffer << file.rdbuf();
        std::string body = buffer.str();
        set_body(body);

        std::stringstream ss;
        ss << body.length();
        add_header("Content-Type", "text/html");
        add_header("Content-Length", ss.str());
        return;
      }
    }
  }

  std::string body = _get_default_error_html(code, phrase, username);
  set_body(body);

  std::stringstream ss;
  ss << body.length();

  add_header("Content-Type", "text/html");
  add_header("Content-Length", ss.str());
}
