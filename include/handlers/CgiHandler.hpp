// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_HANDLERS_CGIHANDLER_HPP_
#define INCLUDE_HANDLERS_CGIHANDLER_HPP_

#include <sys/wait.h>

#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

struct CgiProcess {
  pid_t pid;
  int pipe_in;   // Para enviar el body al CGI (si aplica)
  int pipe_out;  // Para leer la salida del CGI
};

class CgiHandler {
 public:
  // Orthodox Canonical Form
  CgiHandler();
  CgiHandler(const CgiHandler& other);
  CgiHandler& operator=(const CgiHandler& other);
  ~CgiHandler();

  // Modificamos execute_script para que devuelva CgiProcess en lugar de
  // ejecutar la espera síncrona
  bool start_script(const std::string& script_path, const HttpRequest& req,
                    const LocationConfig* loc, CgiProcess& cgi_proc);
  bool parse_cgi_output(const std::string& raw_output, HttpResponse* res) const;

  bool execute_script(const std::string& script_path, const HttpRequest& req,
                      const LocationConfig* loc, HttpResponse* res);

 private:
  friend void test_cgi_env_generation();
  friend void test_cgi_env_allocation();
  friend void test_cgi_ipc_mechanisms();
  friend void test_cgi_interpreter_resolution();

  bool _initialize_stdout_pipe(int stdout_pipe[2]) const;
  FILE* _create_temp_body_file(const HttpRequest& req) const;
  bool _execute_fork(const std::string& script_path, const HttpRequest& req,
                     const LocationConfig* loc, int stdout_pipe[2],
                     FILE* tmp_file, HttpResponse* res) const;
  std::string _get_interpreter(const std::string& script_path,
                               const LocationConfig* loc) const;

  std::vector<std::string> _build_env_vector(const HttpRequest& req,
                                             const LocationConfig* loc) const;
  void _add_core_variables(std::vector<std::string>* env,
                           const HttpRequest& req) const;
  void _add_query_string(std::vector<std::string>* env,
                         const std::string& uri) const;
  void _add_path_info(std::vector<std::string>* env, const HttpRequest& req,
                      const LocationConfig* loc) const;
  void _add_custom_headers(
      std::vector<std::string>* env,
      const std::map<std::string, std::string>& headers) const;

  char** _allocate_env_array(const std::vector<std::string>& env_vec) const;
  void _free_env_array(char** envp) const;
};

#endif  // INCLUDE_HANDLERS_CGIHANDLER_HPP_
