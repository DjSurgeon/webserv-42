// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "http/HttpResponse.hpp"

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

void test_initial_state() {
  std::cout << "[Test] Verifying default constructor state..." << std::endl;
  HttpResponse res;
  std::string s = res.to_string();

  // Default status 200 OK, empty body, no headers
  bool pass = (s.find("HTTP/1.1 200 OK\r\n\r\n") == 0);
  print_result("test_initial_state", pass);
}

void test_canonical_form() {
  std::cout << "[Test] Verifying deep copy and assignment..." << std::endl;
  HttpResponse res1;
  res1.set_status(404, "Not Found");
  res1.add_header("Content-Type", "text/plain");
  res1.set_body("Original Body");

  // Copy Constructor
  HttpResponse res2(res1);
  bool copy_pass = (res2.to_string() == res1.to_string());

  // Assignment Operator
  HttpResponse res3;
  res3 = res1;
  bool assign_pass = (res3.to_string() == res1.to_string());

  // Modify original and check deep copy
  res1.set_body("Modified Body");
  bool deep_pass = (res2.to_string() != res1.to_string());

  print_result("test_canonical_form", copy_pass && assign_pass && deep_pass);
}

void test_serialization() {
  std::cout << "[Test] Verifying serialization (to_string)..." << std::endl;
  HttpResponse res;
  res.set_status(200, "OK");
  res.add_header("Server", "Webserv/1.0");
  res.add_header("Content-Type", "text/html");
  res.set_body("Hello World");

  std::string s = res.to_string();

  bool check_status = (s.find("HTTP/1.1 200 OK\r\n") == 0);
  bool check_header1 = (s.find("Server: Webserv/1.0\r\n") != std::string::npos);
  bool check_header2 =
      (s.find("Content-Type: text/html\r\n") != std::string::npos);
  bool check_body = (s.find("\r\n\r\nHello World") != std::string::npos);

  print_result("test_serialization",
               check_status && check_header1 && check_header2 && check_body);
}

void test_error_generation() {
  std::cout << "[Test] Verifying generate_error_response..." << std::endl;

  // Known code
  HttpResponse res404;
  res404.generate_error_response(404);
  std::string s404 = res404.to_string();
  bool pass404 = (s404.find("404 Not Found") != std::string::npos &&
                  s404.find("Content-Length:") != std::string::npos);

  // Unknown code (fallback to 500)
  HttpResponse res999;
  res999.generate_error_response(999);
  std::string s999 = res999.to_string();
  bool pass999 = (s999.find("500 Internal Server Error") != std::string::npos);

  print_result("test_error_generation", pass404 && pass999);
}

void test_edge_cases() {
  std::cout << "[Test] Verifying massive body and header flood..." << std::endl;

  HttpResponse res;

  // Massive Body (approx 1MB for unit test speed, 10MB in stress script if
  // needed)
  std::string huge_body(1000000, 'A');
  res.set_body(huge_body);
  std::string s_huge = res.to_string();
  bool huge_body_pass = (s_huge.length() > 1000000);

  // Header Flood (1000 headers)
  HttpResponse res_headers;
  for (int i = 0; i < 1000; ++i) {
    std::stringstream ss_k, ss_v;
    ss_k << "X-Header-" << i;
    ss_v << "value-" << i;
    res_headers.add_header(ss_k.str(), ss_v.str());
  }
  std::string s_headers = res_headers.to_string();
  bool header_flood_pass =
      (s_headers.find("X-Header-999:") != std::string::npos);

  print_result("test_edge_cases", huge_body_pass && header_flood_pass);
}

void test_stress() {
  std::cout << "[Test] Stress test: 50,000 response generations..."
            << std::endl;
  for (int i = 0; i < 50000; ++i) {
    HttpResponse res;
    res.generate_error_response(404);
    std::string s = res.to_string();
    if (s.empty()) {
      print_result("test_stress", false);
      return;
    }
  }
  print_result("test_stress", true);
}

void test_http_response_cookies() {
  std::cout << "[Test] Verifying Set-Cookie serialization..." << std::endl;
  HttpResponse res;

  // 1. Single cookie with options
  res.add_cookie("session", "42", "Path=/; HttpOnly");
  // 2. Single cookie with default options
  res.add_cookie("theme", "dark");

  std::string s = res.to_string();

  // Assertions
  bool check1 =
      (s.find("Set-Cookie: session=42; Path=/; HttpOnly\r\n") != std::string::npos);
  bool check2 = (s.find("Set-Cookie: theme=dark\r\n") != std::string::npos);

  // Verification of coexistence (both should be there)
  print_result("test_http_response_cookies", check1 && check2);
}

int main() {
  std::cout << "=== STARTING HTTP RESPONSE TESTS ===\n" << std::endl;

  test_initial_state();
  std::cout << std::endl;
  test_canonical_form();
  std::cout << std::endl;
  test_serialization();
  std::cout << std::endl;
  test_error_generation();
  std::cout << std::endl;
  test_http_response_cookies();
  std::cout << std::endl;
  test_edge_cases();
  std::cout << std::endl;
  test_stress();

  std::cout << "\n=== HTTP RESPONSE TESTS COMPLETED ===" << std::endl;
  return 0;
}
