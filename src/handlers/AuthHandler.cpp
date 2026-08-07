// Copyright 2026 raperez- serjimen
#include "handlers/AuthHandler.hpp"

#include <sstream>
#include <map>

#include "http/SessionManager.hpp"

AuthHandler::AuthHandler() {}
AuthHandler::~AuthHandler() {}

bool AuthHandler::handle_auth_request(const std::string& path,
                                      const HttpRequest& req,
                                      HttpResponse* res) const {
  if (!res) return false;

  // Intercept exact API routes
  if (path.find("/api/login") != std::string::npos &&
      req.get_method() == "POST") {
    _handle_login(req, res);
    return true;
  }
  if (path.find("/api/logout") != std::string::npos &&
      req.get_method() == "POST") {
    _handle_logout(req, res);
    return true;
  }
  if (path.find("/api/profile") != std::string::npos &&
      req.get_method() == "GET") {
    _handle_profile(req, res);
    return true;
  }

  return false;
}

void AuthHandler::_handle_login(const HttpRequest& req,
                                HttpResponse* res) const {
  // Very simple parsing for "username=xyz" in the body
  std::string body = req.get_body();
  std::string username = "Guest";
  
  size_t pos = body.find("username=");
  if (pos != std::string::npos) {
    size_t end = body.find('&', pos);
    if (end == std::string::npos) end = body.length();
    username = body.substr(pos + 9, end - (pos + 9));
  }

  std::string session_id =
      SessionManager::get_instance().create_session(username);

  // Set cookie and respond
  res->set_status(200, "OK");
  res->add_header("Content-Type", "text/html");
  res->add_cookie("session_id", session_id, "Path=/; Max-Age=3600");

  std::string html =
      "<html><body><h1>Logged in successfully as " + username +
      "</h1><a href=\"/api/profile\">View Profile</a></body></html>";
  
  res->set_body(html);
  
  std::stringstream ss;
  ss << html.length();
  res->add_header("Content-Length", ss.str());
}

void AuthHandler::_handle_logout(const HttpRequest& req,
                                 HttpResponse* res) const {
  // Check if there is an active session cookie
  const std::map<std::string, std::string>& cookies = req.get_cookies();
  std::map<std::string, std::string>::const_iterator it =
      cookies.find("session_id");
  
  if (it != cookies.end()) {
    SessionManager::get_instance().destroy_session(it->second);
  }

  // Clear cookie from client
  res->set_status(200, "OK");
  res->add_header("Content-Type", "text/html");
  res->add_cookie("session_id", "",
                  "Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");

  std::string html =
      "<html><body><h1>Logged out successfully.</h1>"
      "<a href=\"/\">Return Home</a></body></html>";
  
  res->set_body(html);
  
  std::stringstream ss;
  ss << html.length();
  res->add_header("Content-Length", ss.str());
}

void AuthHandler::_handle_profile(const HttpRequest& req,
                                  HttpResponse* res) const {
  const std::map<std::string, std::string>& cookies = req.get_cookies();
  std::map<std::string, std::string>::const_iterator it =
      cookies.find("session_id");
  
  if (it != cookies.end()) {
    SessionData* session =
        SessionManager::get_instance().get_session(it->second);
    if (session) {
      res->set_status(200, "OK");
      res->add_header("Content-Type", "text/html");
      
      std::string html =
          "<html><body><h1>Welcome back to your profile, " +
          session->username +
          "!</h1><form method=\"POST\" action=\"/api/logout\"><button "
          "type=\"submit\">Logout</button></form></body></html>";
      res->set_body(html);
      
      std::stringstream ss;
      ss << html.length();
      res->add_header("Content-Length", ss.str());
      return;
    }
  }

  // Unauthorized
  res->set_status(401, "Unauthorized");
  res->add_header("Content-Type", "text/html");
  
  std::string html =
      "<html><body><h1>401 Unauthorized</h1><p>You must log in to view this "
      "page.</p></body></html>";
  res->set_body(html);
  
  std::stringstream ss;
  ss << html.length();
  res->add_header("Content-Length", ss.str());
}
