// Copyright 2026 serjimen vja-nie dlesieur
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "config/Context.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/SessionManager.hpp"

/**
 * @brief Helper utility to get the standard HTTP reason phrase for a given
 * code.
 *
 * @param code The HTTP status code.
 * @return The corresponding reason phrase string.
 */
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
    case 413:
      return "Payload Too Large";
    case 500:
      return "Internal Server Error";
    case 502:
      return "Bad Gateway";
    case 504:
      return "Gateway Timeout";
    default:
      return "Internal Server Error";
  }
}

std::string HttpResponse::_extract_username(const HttpRequest* req) const {
  if (!req) return "";
  const std::map<std::string, std::string>& cookies = req->get_cookies();
  std::map<std::string, std::string>::const_iterator it =
      cookies.find("session_id");
  if (it != cookies.end()) {
    SessionData* sdata = SessionManager::get_instance().get_session(it->second);
    if (sdata) {
      return sdata->username;
    }
  }
  return "";
}

void HttpResponse::_finalize_html_response(const std::string& body) {
  set_body(body);
  std::stringstream ss;
  ss << body.length();
  add_header("Content-Type", "text/html");
  add_header("Content-Length", ss.str());
}

bool HttpResponse::_try_serve_custom_error_page(int code, const Context* ctx) {
  if (!ctx) return false;
  const std::map<int, std::string>& err_pages = ctx->getErrorPages();
  std::map<int, std::string>::const_iterator it = err_pages.find(code);
  if (it != err_pages.end()) {
    std::string err_uri = it->second;
    std::string root = ctx->getRoot();
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
      _finalize_html_response(buffer.str());
      return true;
    }
  }
  return false;
}

/**
 * @brief Generates a default HTML error page.
 *
 * @param code The HTTP status code.
 * @param phrase The reason phrase.
 * @param username An optional username to display in the error message.
 * @return A string containing the default HTML error page.
 */
std::string HttpResponse::_get_default_error_html(
    int code, const std::string& phrase, const std::string& username) const {
  std::stringstream ss;
  ss << "<html><body><h1>" << code << " " << phrase << "</h1>";
  if (!username.empty()) {
    ss << "<p>Lo sentimos, <b>" << username
       << "</b>, este recurso no existe en el servidor.</p>";
  }
  ss << "</body></html>";
  return ss.str();
}

/**
 * @brief Generates an appropriate error response body and headers.
 *
 * Checks for configured error pages in the Context. If none are found,
 * generates a default HTML error page.
 *
 * @param code The HTTP error status code.
 * @param ctx The configuration context (optional).
 * @param req The original HTTP request (optional, used for session info).
 */
void HttpResponse::generate_error_response(int code, const Context* ctx,
                                           const HttpRequest* req) {
  std::string phrase = get_reason_phrase(code);

  if (phrase == "Internal Server Error" && code != 500) {
    code = 500;
  }

  set_status(code, phrase);

  if (ctx && _try_serve_custom_error_page(code, ctx)) {
    return;
  }

  std::string username = _extract_username(req);
  std::string body = _get_default_error_html(code, phrase, username);
  _finalize_html_response(body);
}
