// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HANDLERS_CGIHANDLER_HPP_
#define SRC_HANDLERS_CGIHANDLER_HPP_

#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "http/HttpRequest.hpp"

class CgiHandler {
 public:
  // Orthodox Canonical Form
  CgiHandler();
  CgiHandler(const CgiHandler& other);
  CgiHandler& operator=(const CgiHandler& other);
  ~CgiHandler();

 private:
  std::vector<std::string> _build_env_vector(const HttpRequest& req,
                                             const LocationConfig* loc) const;
  char** _allocate_env_array(const std::vector<std::string>& env_vec) const;
  void _free_env_array(char** envp) const;
};

#endif  // SRC_HANDLERS_CGIHANDLER_HPP_
