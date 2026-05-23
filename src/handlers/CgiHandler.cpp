// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/CgiHandler.hpp"

#include <cstddef>
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
  (void)req;
  (void)loc;
  std::vector<std::string> env;
  // TODO(serjimen): Extract variables (e.g., PATH_INFO, QUERY_STRING) from req
  return env;
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
