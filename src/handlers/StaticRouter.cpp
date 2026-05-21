// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/StaticRouter.hpp"

#include <string>
#include <vector>

StaticRouter::StaticRouter() {}

StaticRouter::StaticRouter(const StaticRouter& other) {
  (void)other;
}

StaticRouter& StaticRouter::operator=(const StaticRouter& other) {
  (void)other;
  return *this;
}

StaticRouter::~StaticRouter() {}

bool StaticRouter::process_route(const HttpRequest& req,
                                 const LocationConfig* loc, HttpResponse& res,
                                 std::string& out_physical_path) const {
  // 1. The Initial Firewall: Null Location Block
  if (loc == NULL) {
    res.generate_error_response(404);
    return false;
  }

  // 2. Method Validation: Principle of Least Privilege
  const std::string& method = req.get_method();
  const std::vector<std::string>& allowed_methods = loc->get_allowed_methods();
  bool method_allowed = false;

  if (allowed_methods.empty()) {
    // If empty, ONLY "GET" is allowed by default
    if (method == "GET") {
      method_allowed = true;
    }
  } else {
    for (size_t i = 0; i < allowed_methods.size(); ++i) {
      if (allowed_methods[i] == method) {
        method_allowed = true;
        break;
      }
    }
  }

  if (!method_allowed) {
    res.generate_error_response(405);
    return false;
  }

  // 3. Path Translation: NGINX Standard Root with Defensive Concatenation
  const std::string& root = loc->get_root();
  const std::string& uri = req.get_uri();

  std::string clean_root = root;
  std::string clean_uri = uri;

  // Prevent double slashes when joining root and uri
  if (!clean_root.empty() && clean_root[clean_root.length() - 1] == '/') {
    clean_root = clean_root.substr(0, clean_root.length() - 1);
  }

  if (clean_uri.empty() || clean_uri[0] != '/') {
    clean_uri = "/" + clean_uri;
  }

  out_physical_path = clean_root + clean_uri;

  return true;
}
