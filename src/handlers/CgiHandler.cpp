// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/CgiHandler.hpp"

#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Orthodox Canonical Form
// -----------------------------------------------------------------------------

CgiHandler::CgiHandler() {}

CgiHandler::CgiHandler(const CgiHandler& other) {
  (void)other;
}

CgiHandler& CgiHandler::operator=(const CgiHandler& other) {
  if (this != &other) {
    // Nothing to assign yet
  }
  return *this;
}

CgiHandler::~CgiHandler() {}

// -----------------------------------------------------------------------------
// Private Methods (Memory & Environment Management)
// -----------------------------------------------------------------------------

std::vector<std::string> CgiHandler::_build_env_vector(
    const HttpRequest& req, const LocationConfig* loc) const {
  (void)loc;
  std::vector<std::string> env;

  _add_core_variables(env, req);
  _add_query_string(env, req.get_uri());
  _add_custom_headers(env, req.get_headers());

  return env;
}

void CgiHandler::_add_core_variables(std::vector<std::string>& env,
                                     const HttpRequest& req) const {
  env.push_back("REQUEST_METHOD=" + req.get_method());
  env.push_back("SERVER_PROTOCOL=HTTP/1.1");
}

void CgiHandler::_add_query_string(std::vector<std::string>& env,
                                   const std::string& uri) const {
  size_t query_pos = uri.find('?');
  if (query_pos != std::string::npos) {
    env.push_back("QUERY_STRING=" + uri.substr(query_pos + 1));
  } else {
    env.push_back("QUERY_STRING=");
  }
}

void CgiHandler::_add_custom_headers(
    std::vector<std::string>& env,
    const std::map<std::string, std::string>& headers) const {
  std::map<std::string, std::string>::const_iterator it;

  it = headers.find("Content-Length");
  if (it != headers.end()) {
    env.push_back("CONTENT_LENGTH=" + it->second);
  }

  it = headers.find("Content-Type");
  if (it != headers.end()) {
    env.push_back("CONTENT_TYPE=" + it->second);
  }

  for (it = headers.begin(); it != headers.end(); ++it) {
    if (it->first == "Content-Length" || it->first == "Content-Type") {
      continue;
    }

    std::string env_key = "HTTP_";
    for (size_t i = 0; i < it->first.length(); ++i) {
      if (it->first[i] == '-') {
        env_key += '_';
      } else {
        env_key += std::toupper(it->first[i]);
      }
    }
    env.push_back(env_key + "=" + it->second);
  }
}

char** CgiHandler::_allocate_env_array(
    const std::vector<std::string>& env_vec) const {
  (void)env_vec;
  // TODO(serjimen): Implement safe string copying to char**
  return NULL;
}

void CgiHandler::_free_env_array(char** envp) const {
  (void)envp;
  // TODO(serjimen): Implement safe deletion of char**
}
