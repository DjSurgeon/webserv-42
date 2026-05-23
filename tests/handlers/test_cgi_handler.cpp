// Copyright 2026 serjimen vja-nie dlesieur
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "handlers/CgiHandler.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

// Helper para buscar variables en el vector de entorno
static bool has_env(const std::vector<std::string>& env,
                    const std::string& expected) {
  for (size_t i = 0; i < env.size(); ++i) {
    if (env[i] == expected) return true;
  }
  return false;
}

void test_cgi_env_generation() {
  // 1. ARRANGE
  CgiHandler handler;
  HttpRequest req;
  req.set_method("POST");
  req.set_uri("/cgi-bin/test.py?user=sergio&token=123");
  req.add_header("Content-Type", "application/json");
  req.add_header("Content-Length", "42");
  req.add_header("X-Custom-Auth", "true-secret");

  // 2. ACT
  std::vector<std::string> env = handler._build_env_vector(req, NULL);

  // 3. ASSERT
  std::cout << "[Test] Verifying CGI environment variable generation..."
            << std::endl;

  // Core Variables
  assert(has_env(env, "REQUEST_METHOD=POST"));
  assert(has_env(env, "SERVER_PROTOCOL=HTTP/1.1"));

  // Query String
  assert(has_env(env, "QUERY_STRING=user=sergio&token=123"));

  // Direct Headers
  assert(has_env(env, "CONTENT_TYPE=application/json"));
  assert(has_env(env, "CONTENT_LENGTH=42"));

  // Custom Meta-Variables (HTTP_ prefix, uppercase, dashes to underscores)
  assert(has_env(env, "HTTP_X_CUSTOM_AUTH=true-secret"));

  print_result("test_cgi_env_generation", true);

  // 1. ARRANGE (Case without query string)
  req.set_uri("/cgi-bin/simple");
  // 2. ACT
  env = handler._build_env_vector(req, NULL);
  // 3. ASSERT
  assert(has_env(env, "QUERY_STRING="));
  print_result("test_cgi_env_no_query_string", true);
}

void test_cgi_env_allocation() {
  // 1. ARRANGE
  CgiHandler handler;
  std::vector<std::string> env_vec;
  env_vec.push_back("VAR1=VALUE1");
  env_vec.push_back("VAR2=VALUE2");

  // 2. ACT
  char** envp = handler._allocate_env_array(env_vec);

  // 3. ASSERT
  std::cout << "[Test] Verifying CGI environment allocation and strcmp..."
            << std::endl;
  assert(envp != NULL);
  assert(std::strcmp(envp[0], "VAR1=VALUE1") == 0);
  assert(std::strcmp(envp[1], "VAR2=VALUE2") == 0);
  assert(envp[2] == NULL);  // Must be NULL terminated for execve

  print_result("test_cgi_env_allocation_logic", true);

  // 4. CLEANUP
  handler._free_env_array(envp);
  print_result("test_cgi_env_free (Valgrind will verify leaks)", true);
}

void test_cgi_ipc_mechanisms() {
  // 1. ARRANGE
  CgiHandler handler;
  int stdout_pipe[2];

  // 2. ACT (Pipe)
  std::cout << "[Test] Verifying CGI stdout pipe initialization..."
            << std::endl;
  bool pipe_success = handler._initialize_stdout_pipe(stdout_pipe);

  // 3. ASSERT (Pipe)
  assert(pipe_success == true);
  assert(stdout_pipe[0] > 0);
  assert(stdout_pipe[1] > 0);
  close(stdout_pipe[0]);
  close(stdout_pipe[1]);
  print_result("test_cgi_stdout_pipe", true);

  // 1. ARRANGE (Temp File)
  HttpRequest req;
  req.set_method("POST");
  req.set_body("CGI INPUT DATA");

  // 2. ACT (Temp File)
  std::cout << "[Test] Verifying Anti-Deadlock temp file creation..."
            << std::endl;
  FILE* tmp_file = handler._create_temp_body_file(req);

  // 3. ASSERT (Temp File)
  assert(tmp_file != NULL);
  // Ensure the offset is back at 0 for child process reading
  assert(std::ftell(tmp_file) == 0);

  // 4. CLEANUP (Closes and deletes file)
  std::fclose(tmp_file);
  print_result("test_cgi_anti_deadlock_tmpfile", true);
}

int main() {
  std::cout << "=== STARTING CGI HANDLER UNIT TESTS ===\n" << std::endl;

  test_cgi_env_generation();
  test_cgi_env_allocation();
  test_cgi_ipc_mechanisms();

  std::cout << "\n=== CGI HANDLER TESTS COMPLETED ===" << std::endl;
  return 0;
}
