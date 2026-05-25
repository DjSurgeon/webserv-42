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
                                 const ServerConfig* server,
                                 const LocationConfig* loc, HttpResponse* res,
                                 std::string* out_physical_path) const {
  if (!_check_null_context(server, loc, res)) {
    return false;
  }

  const Context* ctx = loc ? static_cast<const Context*>(loc)
                           : static_cast<const Context*>(server);

  if (!_validate_method(req, ctx, res)) {
    return false;
  }

  if (!_validate_payload_size(req, ctx, res)) {
    return false;
  }

  _translate_path(req, ctx, out_physical_path);

  return true;
}

bool StaticRouter::_check_null_context(const ServerConfig* server,
                                       const LocationConfig* loc,
                                       HttpResponse* res) const {
  if (loc == NULL && server == NULL) {
    if (res) {
      res->generate_error_response(404);
    }
    return false;
  }
  return true;
}

bool StaticRouter::_validate_method(const HttpRequest& req, const Context* ctx,
                                    HttpResponse* res) const {
  const std::string& method = req.get_method();
  const std::vector<std::string>& allowed_methods = ctx->get_allowed_methods();
  bool method_allowed = false;

  if (allowed_methods.empty()) {
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
    if (res) {
      res->generate_error_response(405);
    }
    return false;
  }

  return true;
}

bool StaticRouter::_validate_payload_size(const HttpRequest& req,
                                          const Context* ctx,
                                          HttpResponse* res) const {
  size_t max_size = ctx->get_client_max_body_size();

  // 0 is interpreted as unlimited body size
  if (max_size > 0 && req.get_body().length() > max_size) {
    if (res) {
      res->generate_error_response(413);
    }
    return false;
  }

  return true;
}

void StaticRouter::_translate_path(const HttpRequest& req, const Context* ctx,
                                   std::string* out_physical_path) const {
  if (!out_physical_path) {
    return;
  }
  const std::string& root = ctx->get_root();
  const std::string& uri = req.get_uri();

  std::string clean_root = root;
  std::string clean_uri = uri;

  if (!clean_root.empty() && clean_root[clean_root.length() - 1] == '/') {
    clean_root = clean_root.substr(0, clean_root.length() - 1);
  }

  if (clean_uri.empty() || clean_uri[0] != '/') {
    clean_uri = "/" + clean_uri;
  }

  *out_physical_path = clean_root + clean_uri;
}
