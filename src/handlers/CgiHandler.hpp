// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HANDLERS_CGIHANDLER_HPP_
#define SRC_HANDLERS_CGIHANDLER_HPP_

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

class CgiHandler {
 public:
  // Orthodox Canonical Form
  CgiHandler();
  CgiHandler(const CgiHandler& other);
  CgiHandler& operator=(const CgiHandler& other);
  ~CgiHandler();

  bool execute_script(const std::string& script_path, const HttpRequest& req,
                      const LocationConfig* loc, HttpResponse& res);

 private:
  friend void test_cgi_env_generation();
  friend void test_cgi_env_allocation();
  friend void test_cgi_ipc_mechanisms();
  friend void test_cgi_interpreter_resolution();

  bool _initialize_stdout_pipe(int stdout_pipe[2]) const;
  FILE* _create_temp_body_file(const HttpRequest& req) const;
  bool _execute_fork(const std::string& script_path, const HttpRequest& req,
                     const LocationConfig* loc, int stdout_pipe[2],
                     FILE* tmp_file, HttpResponse& res) const;
  std::string _get_interpreter(const std::string& script_path,
                               const LocationConfig* loc) const;

  std::vector<std::string> _build_env_vector(const HttpRequest& req,
                                             const LocationConfig* loc) const;
  void _add_core_variables(std::vector<std::string>& env,
                           const HttpRequest& req) const;
  void _add_query_string(std::vector<std::string>& env,
                         const std::string& uri) const;
  void _add_custom_headers(
      std::vector<std::string>& env,
      const std::map<std::string, std::string>& headers) const;

  char** _allocate_env_array(const std::vector<std::string>& env_vec) const;
  void _free_env_array(char** envp) const;
};

#endif  // SRC_HANDLERS_CGIHANDLER_HPP_
