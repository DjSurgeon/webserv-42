// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
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

int main() {
  std::cout << "=== STARTING CGI HANDLER UNIT TESTS ===\n" << std::endl;

  test_cgi_env_generation();

  std::cout << "\n=== CGI HANDLER TESTS COMPLETED ===" << std::endl;
  return 0;
}
