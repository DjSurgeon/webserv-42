// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/StaticRouter.hpp"

#include <sys/stat.h>

#include <string>
#include <vector>
#include <iostream>

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
                                 std::string* out_physical_path,
                                 const Context* active_ctx) const {
  if (!_check_null_context(server, loc, res, active_ctx)) {
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
                                       HttpResponse* res,
                                       const Context* ctx) const {
  if (loc == NULL && server == NULL) {
    if (res) {
      res->generate_error_response(404, ctx, NULL);
    }
    return false;
  }
  return true;
}

bool StaticRouter::_validate_method(const HttpRequest& req, const Context* ctx,
                                    HttpResponse* res) const {
  const std::string& method = req.get_method();
  const std::vector<std::string>& allowed_methods = ctx->getAllowedMethods();
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
      res->generate_error_response(405, ctx, &req);
    }
    return false;
  }

  return true;
}

bool StaticRouter::_validate_payload_size(const HttpRequest& req,
                                          const Context* ctx,
                                          HttpResponse* res) const {
  size_t max_size = ctx->getClientMaxBodySize();

  // 0 is interpreted as unlimited body size
  if (max_size > 0 && req.get_body().length() > max_size) {
    if (res) {
      res->generate_error_response(413, ctx, &req);
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
  const std::string& root = ctx->getRoot();
  const std::string& uri = req.get_uri();

  std::string clean_root = root;
  std::string clean_uri = uri;

  const LocationConfig* location =
    dynamic_cast<const LocationConfig*>(ctx);

  if (location) {
    const std::string& prefix = location->getPath();
    if (clean_uri.find(prefix) == 0)
      clean_uri.erase(0, prefix.length());
  }

  if (!clean_root.empty() && clean_root[clean_root.length() - 1] == '/') {
    clean_root = clean_root.substr(0, clean_root.length() - 1);
  }

  if (clean_uri.empty() || clean_uri[0] != '/') {
    clean_uri = "/" + clean_uri;
  }

  *out_physical_path = clean_root + clean_uri;

  struct stat st;
  if (stat(out_physical_path->c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
    const std::vector<std::string>& index_files = ctx->getIndexFiles();
    std::string test_path = "";
    for (size_t i = 0; i < index_files.size(); ++i) {
      std::string base_path = *out_physical_path;
      if (!base_path.empty() && base_path[base_path.length() - 1] != '/') {
        base_path += "/";
      }
      test_path = base_path + index_files[i];
      struct stat idx_st;
      if (stat(test_path.c_str(), &idx_st) == 0 && S_ISREG(idx_st.st_mode)) {
        *out_physical_path = test_path;
        break;
      }
    }
    *out_physical_path = test_path;
  }

  // Internationalization (i18n) Check
  if (out_physical_path->length() >= 5 &&
      out_physical_path->substr(out_physical_path->length() - 5) == ".html") {
    std::vector<LanguageWeight> langs = req.get_accepted_languages();
    for (size_t i = 0; i < langs.size(); ++i) {
      std::string lang_path = *out_physical_path + "." + langs[i].lang;
      struct stat lang_st;

      // Try exact match (e.g. index.html.es-ES or index.html.en)
      if (stat(lang_path.c_str(), &lang_st) == 0 && S_ISREG(lang_st.st_mode)) {
        *out_physical_path = lang_path;
        break;
      }

      // Try short match (e.g. es-ES -> es)
      if (langs[i].lang.length() > 2 && langs[i].lang[2] == '-') {
        std::string short_lang = langs[i].lang.substr(0, 2);
        std::string short_lang_path = *out_physical_path + "." + short_lang;
        if (stat(short_lang_path.c_str(), &lang_st) == 0 &&
            S_ISREG(lang_st.st_mode)) {
          *out_physical_path = short_lang_path;
          break;
        }
      }
    }
  }
}
