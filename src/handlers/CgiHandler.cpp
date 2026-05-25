// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/CgiHandler.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
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
                                const HttpRequest& req,
                                const LocationConfig* loc, HttpResponse& res) {
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
  return _execute_fork(script_path, req, loc, stdout_pipe, tmp_file, res);
}

bool CgiHandler::parse_cgi_output(const std::string& raw_output,
                                  HttpResponse& res) const {
  size_t boundary_pos = raw_output.find("\r\n\r\n");
  size_t boundary_len = 4;

  if (boundary_pos == std::string::npos) {
    boundary_pos = raw_output.find("\n\n");
    boundary_len = 2;
  }

  if (boundary_pos == std::string::npos) {
    // Malformed CGI output, no headers found
    res.generate_error_response(502);
    return false;
  }

  // Default status 200 OK
  res.set_status(200, "OK");

  std::string headers_str = raw_output.substr(0, boundary_pos);
  std::string body_str = raw_output.substr(boundary_pos + boundary_len);

  // Parse headers
  size_t start = 0;
  while (start < headers_str.length()) {
    size_t end = headers_str.find('\n', start);
    if (end == std::string::npos) {
      end = headers_str.length();
    }

    std::string line = headers_str.substr(start, end - start);
    if (!line.empty() && line[line.length() - 1] == '\r') {
      line.erase(line.length() - 1);
    }

    size_t colon_pos = line.find(':');
    if (colon_pos != std::string::npos) {
      std::string key = line.substr(0, colon_pos);
      std::string value = line.substr(colon_pos + 1);

      // Trim leading spaces from value
      size_t value_start = 0;
      while (value_start < value.length() &&
             (value[value_start] == ' ' || value[value_start] == '\t')) {
        value_start++;
      }
      value = value.substr(value_start);

      // Check for Status pseudo-header (case-insensitive)
      std::string lower_key = key;
      for (size_t i = 0; i < lower_key.length(); ++i) {
        lower_key[i] = std::tolower(lower_key[i]);
      }

      if (lower_key == "status") {
        size_t space_pos = value.find(' ');
        int code = 200;
        std::string phrase = "OK";
        if (space_pos != std::string::npos) {
          std::stringstream code_ss(value.substr(0, space_pos));
          code_ss >> code;
          phrase = value.substr(space_pos + 1);
        } else {
          std::stringstream code_ss(value);
          code_ss >> code;
          phrase = "";
        }
        res.set_status(code, phrase);
      } else {
        res.add_header(key, value);
      }
    }

    start = end + 1;
  }

  res.set_body(body_str);

  // Calculate and set exact Content-Length
  std::stringstream ss;
  ss << body_str.length();
  res.add_header("Content-Length", ss.str());

  return true;
}


std::string CgiHandler::_get_interpreter(const std::string& script_path,
                                         const LocationConfig* loc) const {
  if (loc != NULL && !loc->get_cgi_path().empty()) {
    return loc->get_cgi_path();
  }
  size_t dot_pos = script_path.find_last_of('.');
  if (dot_pos != std::string::npos) {
    std::string ext = script_path.substr(dot_pos);
    if (ext == ".py") return "/usr/bin/python3";
    if (ext == ".php") return "/usr/bin/php-cgi";
  }
  return "";
}

// -----------------------------------------------------------------------------
// Private Methods (Memory & Environment Management)
// -----------------------------------------------------------------------------

bool CgiHandler::_execute_fork(const std::string& script_path,
                               const HttpRequest& req,
                               const LocationConfig* loc, int stdout_pipe[2],
                               FILE* tmp_file, HttpResponse& res) const {
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
    close(stdout_pipe[0]);  // Close unused read end

    // Redirect STDOUT
    if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1) {
      std::exit(EXIT_FAILURE);
    }
    close(stdout_pipe[1]);  // Close original descriptor

    // Redirect STDIN
    if (tmp_file != NULL) {
      int tmp_fd = fileno(tmp_file);
      if (dup2(tmp_fd, STDIN_FILENO) == -1) {
        std::exit(EXIT_FAILURE);
      }
      fclose(tmp_file);  // Closes original descriptor
    } else {
      // Nginx-like behavior: redirect STDIN to /dev/null if no body
      int dev_null = open("/dev/null", O_RDONLY);
      if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        close(dev_null);
      }
    }

    // Build Environment
    std::vector<std::string> env_vec = _build_env_vector(req, loc);
    char** envp = _allocate_env_array(env_vec);

    // Resolve Interpreter
    std::string interpreter = _get_interpreter(script_path, loc);
    char* argv[3];
    if (!interpreter.empty()) {
      argv[0] = const_cast<char*>(interpreter.c_str());
      argv[1] = const_cast<char*>(script_path.c_str());
      argv[2] = NULL;
    } else {
      argv[0] = const_cast<char*>(script_path.c_str());
      argv[1] = NULL;
    }

    // Execute
    execve(argv[0], argv, envp);

    // Fatal Fallback (Kill-Switch)
    std::cerr << "CGI execve failed: " << std::strerror(errno) << std::endl;
    _free_env_array(envp);
    std::exit(
        EXIT_FAILURE);  // Prevent child from returning to caller/test suite
  } else {
    // Parent Process
    if (tmp_file != NULL) {
      fclose(tmp_file);
    }
    // TODO(serjimen): Register stdout_pipe[0] with EventLoop
    // For now, in tests, we must close stdout_pipe[1] to avoid leaks
    close(stdout_pipe[1]);
    // And close stdout_pipe[0] because we are not reading it yet
    close(stdout_pipe[0]);
    waitpid(pid, NULL, 0);  // Clean up child process (synchronous for tests)
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
