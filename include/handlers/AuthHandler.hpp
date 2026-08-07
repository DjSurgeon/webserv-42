// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_HANDLERS_AUTHHANDLER_HPP_
#define INCLUDE_HANDLERS_AUTHHANDLER_HPP_

#include <string>

#include "config/LocationConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

class AuthHandler {
 public:
  AuthHandler();
  ~AuthHandler();

  /**
   * @brief Handles authentication-related API requests.
   *
   * Returns true if the request was an auth API route and was handled.
   * Returns false if the request should be processed by standard handlers.
   */
  bool handle_auth_request(const std::string& path, const HttpRequest& req,
                           HttpResponse* res) const;

 private:
  void _handle_login(const HttpRequest& req, HttpResponse* res) const;
  void _handle_logout(const HttpRequest& req, HttpResponse* res) const;
  void _handle_profile(const HttpRequest& req, HttpResponse* res) const;
};

#endif  // INCLUDE_HANDLERS_AUTHHANDLER_HPP_
