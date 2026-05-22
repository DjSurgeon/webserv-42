// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>

#include "http/HttpRequest.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_initial_state() {
  std::cout << "[Test] Verifying default constructor state..." << std::endl;
  HttpRequest req;

  bool pass = true;
  if (!req.get_method().empty()) pass = false;
  if (!req.get_uri().empty()) pass = false;
  if (!req.get_version().empty()) pass = false;
  if (!req.get_body().empty()) pass = false;
  if (!req.get_headers().empty()) pass = false;

  print_result("test_initial_state", pass);
}

void test_setters_and_getters() {
  std::cout << "[Test] Verifying setters and getters..." << std::endl;
  HttpRequest req;

  req.set_method("POST");
  req.set_uri("/login?user=admin");
  req.set_version("HTTP/1.1");
  req.set_body("username=admin&password=123");
  req.add_header("Host", "localhost:8080");
  req.add_header("Content-Type", "application/x-www-form-urlencoded");
  req.add_header("Content-Length", "27");

  bool pass = true;
  if (req.get_method() != "POST") {
    std::cerr << "Mismatch method: " << req.get_method() << std::endl;
    pass = false;
  }
  if (req.get_uri() != "/login?user=admin") {
    std::cerr << "Mismatch URI: " << req.get_uri() << std::endl;
    pass = false;
  }
  if (req.get_version() != "HTTP/1.1") {
    std::cerr << "Mismatch version: " << req.get_version() << std::endl;
    pass = false;
  }
  if (req.get_body() != "username=admin&password=123") {
    std::cerr << "Mismatch body: " << req.get_body() << std::endl;
    pass = false;
  }

  const std::map<std::string, std::string>& headers = req.get_headers();
  if (headers.size() != 3) {
    std::cerr << "Mismatch headers size: " << headers.size() << std::endl;
    pass = false;
  } else {
    std::map<std::string, std::string>::const_iterator it =
        headers.find("host");
    if (it == headers.end() || it->second != "localhost:8080") {
      std::cerr << "Mismatch Host header" << std::endl;
      pass = false;
    }
    it = headers.find("content-type");
    if (it == headers.end() ||
        it->second != "application/x-www-form-urlencoded") {
      std::cerr << "Mismatch Content-Type header" << std::endl;
      pass = false;
    }
    it = headers.find("content-length");
    if (it == headers.end() || it->second != "27") {
      std::cerr << "Mismatch Content-Length header" << std::endl;
      pass = false;
    }
  }

  print_result("test_setters_and_getters", pass);
}

void test_clear() {
  std::cout << "[Test] Verifying clear functionality..." << std::endl;
  HttpRequest req;

  req.set_method("GET");
  req.set_uri("/index.html");
  req.set_version("HTTP/1.1");
  req.set_body("Some content");
  req.add_header("Host", "localhost");

  req.clear();

  bool pass = true;
  if (!req.get_method().empty()) pass = false;
  if (!req.get_uri().empty()) pass = false;
  if (!req.get_version().empty()) pass = false;
  if (!req.get_body().empty()) pass = false;
  if (!req.get_headers().empty()) pass = false;

  print_result("test_clear", pass);
}

int main() {
  std::cout << "=== STARTING HTTP REQUEST TESTS ===\n" << std::endl;

  test_initial_state();
  std::cout << std::endl;
  test_setters_and_getters();
  std::cout << std::endl;
  test_clear();

  std::cout << "\n=== HTTP REQUEST TESTS COMPLETED ===" << std::endl;
  return 0;
}
