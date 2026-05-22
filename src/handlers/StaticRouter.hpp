// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HANDLERS_STATICROUTER_HPP_
#define SRC_HANDLERS_STATICROUTER_HPP_

#include <string>

#include "config/LocationConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

class StaticRouter {
 public:
  // Orthodox Canonical Form
  StaticRouter();
  StaticRouter(const StaticRouter& other);
  StaticRouter& operator=(const StaticRouter& other);
  ~StaticRouter();

  // Core Routing Method
  bool process_route(const HttpRequest& req, const LocationConfig* loc,
                     HttpResponse* res, std::string* out_physical_path) const;

 private:
  bool _check_null_location(const LocationConfig* loc, HttpResponse* res) const;
  bool _validate_method(const HttpRequest& req, const LocationConfig* loc,
                        HttpResponse* res) const;
  void _translate_path(const HttpRequest& req, const LocationConfig* loc,
                       std::string* out_physical_path) const;
};

#endif  // SRC_HANDLERS_STATICROUTER_HPP_
