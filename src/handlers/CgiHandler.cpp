// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/CgiHandler.hpp"

#include <cctype>
#include <cstddef>
#include <cstring>
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

  it = headers.find("content-length");
  if (it != headers.end()) {
    env.push_back("CONTENT_LENGTH=" + it->second);
  }

  it = headers.find("content-type");
  if (it != headers.end()) {
    env.push_back("CONTENT_TYPE=" + it->second);
  }

  for (it = headers.begin(); it != headers.end(); ++it) {
    if (it->first == "content-length" || it->first == "content-type") {
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
  size_t size = env_vec.size();
  char** envp = new char*[size + 1];

  for (size_t i = 0; i < size; ++i) {
    envp[i] = new char[env_vec[i].length() + 1];
    std::strcpy(envp[i], env_vec[i].c_str());
  }
  envp[size] = NULL;

  return envp;
}

void CgiHandler::_free_env_array(char** envp) const {
  if (envp == NULL) {
    return;
  }

  for (size_t i = 0; envp[i] != NULL; ++i) {
    delete[] envp[i];
  }
  delete[] envp;
}
