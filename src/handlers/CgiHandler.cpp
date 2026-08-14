// Copyright 2026 raperez- serjimen
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
#include "http/SessionManager.hpp"
#include "network/EventLoop.hpp"

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

bool CgiHandler::parse_cgi_output(const std::string& raw_output,
                                  HttpResponse* res,
                                  size_t boundary_pos) const {
  if (!res) return false;

  // Default status 200 OK
  res->set_status(200, "OK");

  std::string headers_str = raw_output.substr(0, boundary_pos);

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
        res->set_status(code, phrase);
      } else if (lower_key == "set-cookie") {
        res->add_cookie(value);
      } else if (lower_key == "x-create-session") {
        std::string new_session =
            SessionManager::get_instance().create_session(value);
        res->add_cookie("session_id", new_session, "Path=/");
      } else {
        res->add_header(key, value);
      }
    }

    start = end + 1;
  }

  return true;
}

std::string CgiHandler::_get_interpreter(const std::string& script_path,
                                         const LocationConfig* loc) const {
  if (loc != NULL && !loc->getCgiPath().empty()) {
    return loc->getCgiPath();
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

  _add_core_variables(&env, req);
  _add_query_string(&env, req.get_uri());
  _add_path_info(&env, req, loc);
  _add_custom_headers(&env, req.get_headers());

  // Authentication Bridge
  const std::map<std::string, std::string>& cookies = req.get_cookies();
  std::map<std::string, std::string>::const_iterator cookie_it =
      cookies.find("session_id");
  if (cookie_it != cookies.end()) {
    SessionData* sdata =
        SessionManager::get_instance().get_session(cookie_it->second);
    if (sdata != NULL) {
      env.push_back("AUTH_USER=" + sdata->username);
    }
  }

  return env;
}

void CgiHandler::_add_core_variables(std::vector<std::string>* env,
                                     const HttpRequest& req) const {
  if (!env) return;
  env->push_back("REQUEST_METHOD=" + req.get_method());
  env->push_back("SERVER_PROTOCOL=HTTP/1.1");
}

void CgiHandler::_add_query_string(std::vector<std::string>* env,
                                   const std::string& uri) const {
  if (!env) return;
  size_t query_pos = uri.find('?');
  if (query_pos != std::string::npos) {
    env->push_back("QUERY_STRING=" + uri.substr(query_pos + 1));
  } else {
    env->push_back("QUERY_STRING=");
  }
}

void CgiHandler::_add_path_info(std::vector<std::string>* env,
                                const HttpRequest& req,
                                const LocationConfig* loc) const {
  if (!env || !loc) return;

  std::string uri = req.get_uri();

  // Eliminar la query string
  size_t query_pos = uri.find('?');
  if (query_pos != std::string::npos) uri.erase(query_pos);

  std::string script_name = uri;
  std::string path_info;

  const std::vector<std::string>& cgi_exts = loc->getCgiExtensions();

  size_t last_slash = uri.find_last_of('/');
  if (last_slash == std::string::npos) last_slash = 0;
  for (std::vector<std::string>::const_iterator it = cgi_exts.begin();
       it != cgi_exts.end(); ++it) {
    size_t ext_pos = uri.find(*it, last_slash);
    if (ext_pos != std::string::npos) {
      ext_pos += it->length();
      script_name = uri.substr(0, ext_pos);
      if (ext_pos < uri.length()) path_info = uri.substr(ext_pos);
      break;
    }
  }

  std::string location = loc->getPath();

  if (script_name.find(location) == 0) script_name.erase(0, location.length());

  if (script_name.empty() || script_name[0] != '/') script_name.insert(0, "/");

  env->push_back("SCRIPT_NAME=" + script_name);
  env->push_back("PATH_INFO=" + script_name);
  env->push_back("REQUEST_URI=" + script_name);
}

void CgiHandler::_add_custom_headers(
    std::vector<std::string>* env,
    const std::map<std::string, std::string>& headers) const {
  if (!env) return;
  std::map<std::string, std::string>::const_iterator it;

  it = headers.find("content-length");
  if (it != headers.end()) {
    env->push_back("CONTENT_LENGTH=" + it->second);
  }

  it = headers.find("content-type");
  if (it != headers.end()) {
    env->push_back("CONTENT_TYPE=" + it->second);
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
    env->push_back(env_key + "=" + it->second);
  }
}

char** CgiHandler::_allocate_env_array(
    const std::vector<std::string>& env_vec) const {
  size_t size = env_vec.size();
  char** envp = new char*[size + 1];

  for (size_t i = 0; i < size; ++i) {
    envp[i] = new char[env_vec[i].length() + 1];
    // Use manual copy loop for C++98 and to avoid strcpy lint warning
    for (size_t j = 0; j < env_vec[i].length(); ++j) {
      envp[i][j] = env_vec[i][j];
    }
    envp[i][env_vec[i].length()] = '\0';
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

bool CgiHandler::start_script(const std::string& script_path,
                              const HttpRequest& req, const LocationConfig* loc,
                              CgiProcess& cgi_proc,
                              const std::map<int, CgiTask*>& _cgiOutMap,
                              const std::map<int, CgiTask*>& _cgiInMap) {
  int stdout_pipe[2];
  int stdin_pipe[2] = {-1, -1};

  // 1. Pipe para leer salida del CGI
  if (pipe(stdout_pipe) == -1) return false;
  fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);

  // 2. Si hay body, crear Pipe para enviar body al CGI de forma asíncrona
  if (!req.get_body().empty()) {
    if (pipe(stdin_pipe) == -1) {
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);
      return false;
    }
    fcntl(stdin_pipe[1], F_SETFL, O_NONBLOCK);
  }

  pid_t pid = fork();
  if (pid < 0) {
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    if (stdin_pipe[0] != -1) {
      close(stdin_pipe[0]);
      close(stdin_pipe[1]);
    }
    return false;
  } else if (pid == 0) {  // Proceso Hijo
    for (std::map<int, CgiTask*>::const_iterator it = _cgiOutMap.begin();
         it != _cgiOutMap.end(); ++it) {
      close(it->first);
    }
    for (std::map<int, CgiTask*>::const_iterator it = _cgiInMap.begin();
         it != _cgiInMap.end(); ++it) {
      close(it->first);
    }

    close(stdout_pipe[0]);
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[1]);

    if (stdin_pipe[0] != -1) {
      close(stdin_pipe[1]);
      dup2(stdin_pipe[0], STDIN_FILENO);
      close(stdin_pipe[0]);
    } else {
      int dev_null = open("/dev/null", O_RDONLY);
      if (dev_null != -1) {
        dup2(dev_null, STDIN_FILENO);
        close(dev_null);
      }
    }

    std::vector<std::string> env_vec = _build_env_vector(req, loc);
    char** envp = _allocate_env_array(env_vec);
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

    execve(argv[0], argv, envp);
    _free_env_array(envp);
    std::exit(EXIT_FAILURE);
  }

  // Proceso Padre
  close(stdout_pipe[1]);                          // Cierra extremo de escritura
  if (stdin_pipe[0] != -1) close(stdin_pipe[0]);  // Cierra extremo de lectura

  cgi_proc.pid = pid;
  cgi_proc.pipe_out = stdout_pipe[0];
  cgi_proc.pipe_in = stdin_pipe[1];

  return true;
}
