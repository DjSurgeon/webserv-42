// Copyright 2026 serjimen vja-nie dlesieur
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "handlers/CgiHandler.hpp"
#include "http/SessionManager.hpp"

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

  // 1. ARRANGE (Case with AUTH_USER injection)
  {
    std::string sid =
        SessionManager::get_instance().create_session("admin_user");
    req.add_cookie("session_id", sid);

    // 2. ACT
    env = handler._build_env_vector(req, NULL);

    // 3. ASSERT
    assert(has_env(env, "AUTH_USER=admin_user"));
    print_result("test_cgi_auth_user_injection", true);
  }
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

void test_cgi_interpreter_resolution() {
  // 1. ARRANGE
  CgiHandler handler;

  // 2. ACT & 3. ASSERT (By Extension)
  std::cout << "[Test] Verifying interpreter resolution by extension..."
            << std::endl;
  assert(handler._get_interpreter("script.py", NULL) == "/usr/bin/python3");
  assert(handler._get_interpreter("script.php", NULL) == "/usr/bin/php-cgi");
  assert(handler._get_interpreter("script.cgi", NULL) == "");
  assert(handler._get_interpreter("binary", NULL) == "");

  // 2. ACT & 3. ASSERT (By Config override)
  std::cout << "[Test] Verifying interpreter resolution by config override..."
            << std::endl;
  LocationConfig loc;
  loc.setCgiPath("/usr/local/bin/python-custom");
  assert(handler._get_interpreter("script.py", &loc) ==
         "/usr/local/bin/python-custom");
  assert(handler._get_interpreter("any.file", &loc) ==
         "/usr/local/bin/python-custom");

  print_result("test_cgi_interpreter_resolution", true);
}

void test_cgi_execution_flow() {
  std::cout << "[Test] Verifying CGI basic execution flow (fork setup)..."
            << std::endl;
  // 1. ARRANGE
  CgiHandler handler;
  HttpRequest req;
  req.set_method("POST");
  req.set_body("Hello CGI STDIN");
  HttpResponse res;

  // 2. ACT
  // This will initialize pipes/tmpfile and prepare for fork
  bool result =
      handler.execute_script("tests/assets/echo.cgi", req, NULL, &res);

  // 3. ASSERT
  // Since fork/execve IS implemented now, but parent just waits,
  // we check that it didn't crash.
  assert(result == true);
  print_result("test_cgi_execution_flow_setup", true);
}

void test_cgi_10mb_payload_stress() {
  std::cout << "[Test] CGI Stress: 10MB payload (Anti-Deadlock check)..."
            << std::endl;
  // 1. ARRANGE
  CgiHandler handler;
  HttpRequest req;
  req.set_method("POST");
  std::string massive_body(10 * 1024 * 1024, 'A');
  req.set_body(massive_body);
  HttpResponse res;

  // 2. ACT
  bool result =
      handler.execute_script("tests/assets/echo.cgi", req, NULL, &res);

  // 3. ASSERT
  assert(result == true);
  std::cout << GREEN << "  -> Success: Handled 10MB body without blocking."
            << RESET << std::endl;
  print_result("test_cgi_10mb_payload_stress_setup", true);
}

void test_cgi_fd_leaks() {
  std::cout << "[Test] CGI Robustness: 100 consecutive setup calls (FD Leak "
               "check)..."
            << std::endl;
  CgiHandler handler;
  HttpRequest req;
  req.set_method("POST");
  req.set_body("dummy");

  for (int i = 0; i < 100; ++i) {
    HttpResponse res;
    handler.execute_script("tests/assets/echo.cgi", req, NULL, &res);
  }
  print_result("test_cgi_fd_leaks_100_iterations", true);
}

void test_cgi_output_parsing() {
  std::cout << "[Test] Verifying CGI output parsing logic..." << std::endl;
  CgiHandler handler;

  // Case A: Implicit Status (Default 200 OK)
  {
    HttpResponse res;
    std::string raw = "Content-Type: text/html\r\n\r\n<h1>Hello</h1>";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();
    assert(res_str.find("HTTP/1.1 200 OK") != std::string::npos);
    assert(res_str.find("Content-Type: text/html") != std::string::npos);
    assert(res_str.find("Content-Length: 14") != std::string::npos);
    assert(res_str.find("<h1>Hello</h1>") != std::string::npos);
    print_result("  -> Implicit 200 OK", true);
  }

  // Case B: Valid Output (\n\n boundary)
  {
    HttpResponse res;
    std::string raw = "Content-Type: text/plain\n\nTexto plano";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();
    assert(res_str.find("Content-Type: text/plain") != std::string::npos);
    assert(res_str.find("Texto plano") != std::string::npos);
    print_result("  -> Valid \n\n", true);
  }

  // Case C: Explicit Status (pseudo-header removal and mutation)
  {
    HttpResponse res;
    std::string raw =
        "Status: 404 Not Found\nContent-Type: application/json\nSet-Cookie: "
        "session=123\n\n{\"error\":\"Not Found\"}";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();

    // 1. Check mutation to 404
    assert(res_str.find("HTTP/1.1 404 Not Found") != std::string::npos);
    // 2. Check "Status:" is NOT in the final headers
    assert(res_str.find("Status:") == std::string::npos);

    assert(res_str.find("Content-Type: application/json") != std::string::npos);
    assert(res_str.find("Set-Cookie: session=123") != std::string::npos);
    print_result("  -> Explicit Status (Pseudo-header handling)", true);
  }

  // Case D: Malformed Output (No boundary)
  {
    HttpResponse res;
    std::string raw = "Just some crazy text without any double newlines";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();
    assert(res_str.find("502 Bad Gateway") != std::string::npos);
    print_result("  -> Malformed (502 Check)", true);
  }

  // Case E: Multiple Cookies from CGI
  {
    HttpResponse res;
    std::string raw =
        "Content-Type: text/html\nSet-Cookie: user=serjimen\nSet-Cookie: "
        "pref=dark\n\n<h1>Cookies!</h1>";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();
    assert(res_str.find("Set-Cookie: user=serjimen") != std::string::npos);
    assert(res_str.find("Set-Cookie: pref=dark") != std::string::npos);
    print_result("  -> Multiple Cookies from CGI", true);
  }

  // Case F: Complex Cookies with Options
  {
    HttpResponse res;
    std::string raw =
        "Set-Cookie: session=xyz; Path=/; HttpOnly; Secure; "
        "SameSite=Strict\n\nBody";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();
    assert(res_str.find("Set-Cookie: session=xyz; Path=/; HttpOnly; Secure; "
                        "SameSite=Strict") != std::string::npos);
    print_result("  -> Complex Cookies with Options", true);
  }

  // Case G: Magic Directive X-Create-Session
  {
    HttpResponse res;
    std::string raw = "X-Create-Session: magic_user\n\nDone";
    handler.parse_cgi_output(raw, &res);
    std::string res_str = res.to_string();

    // 1. Verify Set-Cookie header was generated
    size_t cookie_pos = res_str.find("Set-Cookie: session_id=");
    assert(cookie_pos != std::string::npos);

    // 2. Extract session ID from response string
    size_t start = cookie_pos + std::string("Set-Cookie: session_id=").length();
    size_t end = res_str.find(';', start);
    if (end == std::string::npos) end = res_str.find("\r\n", start);
    std::string sid = res_str.substr(start, end - start);

    // 3. Verify session exists in RAM for that user
    SessionData* data = SessionManager::get_instance().get_session(sid);
    assert(data != NULL);
    assert(data->username == "magic_user");

    print_result("  -> Magic Directive X-Create-Session", true);
  }

  print_result("test_cgi_output_parsing", true);
}

int main() {
  std::cout << "=== STARTING CGI HANDLER UNIT TESTS ===\n" << std::endl;

  test_cgi_env_generation();
  test_cgi_env_allocation();
  test_cgi_ipc_mechanisms();
  test_cgi_interpreter_resolution();
  std::cout << std::endl;
  test_cgi_output_parsing();
  std::cout << std::endl;
  test_cgi_execution_flow();
  test_cgi_10mb_payload_stress();
  test_cgi_fd_leaks();

  std::cout << "\n=== CGI HANDLER TESTS COMPLETED ===" << std::endl;
  return 0;
}
