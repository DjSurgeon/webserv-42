// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/CgiHandler.hpp"

#include <sys/types.h>
#include <unistd.h>

#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "http/HttpResponse.hpp"

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
// Public Methods
// -----------------------------------------------------------------------------

bool CgiHandler::execute_script(const std::string& script_path,
                                const HttpRequest& req, HttpResponse& res) {
  (void)script_path;
  int stdout_pipe[2];
  FILE* tmp_file = NULL;

  // 1. Establish POSIX pipe for IPC (CGI to Server)
  if (!_initialize_stdout_pipe(stdout_pipe)) {
    res.generate_error_response(500);
    return true;  // Handled
  }

  // 2. Anti-Deadlock: Create temporary file for POST body
  if (req.get_method() == "POST" && !req.get_body().empty()) {
    tmp_file = _create_temp_body_file(req);
    if (!tmp_file) {
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);
      res.generate_error_response(500);
      return true;  // Handled
    }
  }

  // 3. Clone process and execute
  return _execute_fork(script_path, req, stdout_pipe, tmp_file, res);
}

// -----------------------------------------------------------------------------
// Private Methods (Memory & Environment Management)
// -----------------------------------------------------------------------------

bool CgiHandler::_execute_fork(const std::string& script_path,
                               const HttpRequest& req, int stdout_pipe[2],
                               FILE* tmp_file, HttpResponse& res) const {
  (void)script_path;
  (void)req;
  pid_t pid = fork();

  if (pid < 0) {
    // Fork failed: close all resources
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    if (tmp_file != NULL) {
      fclose(tmp_file);
    }
    res.generate_error_response(500);
    return true;  // Handled
  } else if (pid == 0) {
    // Child Process
    // TODO(serjimen): dup2 and execve
    if (tmp_file != NULL) {
      fclose(tmp_file); // Temporary closure until dup2 is implemented
    }
  } else {
    // Parent Process
    if (tmp_file != NULL) {
      fclose(tmp_file);
    }
    // TODO(serjimen): Register stdout_pipe[0] with EventLoop
  }

  return false;
}

bool CgiHandler::_initialize_stdout_pipe(int stdout_pipe[2]) const {
  if (pipe(stdout_pipe) == -1) {
    return false;
  }
  return true;
}

FILE* CgiHandler::_create_temp_body_file(const HttpRequest& req) const {
  FILE* tmp_file = tmpfile();
  if (!tmp_file) {
    return NULL;
  }

  int tmp_fd = fileno(tmp_file);
  const std::string& body = req.get_body();

  ssize_t bytes_written = write(tmp_fd, body.c_str(), body.length());
  if (bytes_written == -1 ||
      static_cast<size_t>(bytes_written) != body.length()) {
    fclose(tmp_file);
    return NULL;
  }

  if (lseek(tmp_fd, 0, SEEK_SET) == -1) {
    fclose(tmp_file);
    return NULL;
  }

  return tmp_file;
}

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
