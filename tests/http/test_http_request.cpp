// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <map>
#include <string>

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

void test_accept_language_parsing() {
  std::cout << "[Test] Verifying Accept-Language parsing..." << std::endl;
  HttpRequest req;
  req.add_header("Accept-Language", "fr, en;q=0.9, es;q=0.2");

  std::vector<LanguageWeight> langs = req.get_accepted_languages();

  bool pass = true;
  if (langs.size() != 3) {
    std::cerr << "Expected 3 languages, got " << langs.size() << std::endl;
    pass = false;
  }
  if (pass && (langs[0].lang != "fr" || langs[0].q != 1.0)) {
    std::cerr << "Expected fr with q=1.0, got " << langs[0].lang
              << " with q=" << langs[0].q << std::endl;
    pass = false;
  }
  if (pass && (langs[1].lang != "en" || langs[1].q != 0.9)) {
    std::cerr << "Expected en with q=0.9, got " << langs[1].lang
              << " with q=" << langs[1].q << std::endl;
    pass = false;
  }
  if (pass && (langs[2].lang != "es" || langs[2].q != 0.2)) {
    std::cerr << "Expected es with q=0.2, got " << langs[2].lang
              << " with q=" << langs[2].q << std::endl;
    pass = false;
  }

  print_result("test_accept_language_parsing", pass);
}

int main() {
  std::cout << "=== STARTING HTTP REQUEST TESTS ===\n" << std::endl;

  test_initial_state();
  std::cout << std::endl;
  test_setters_and_getters();
  std::cout << std::endl;
  test_clear();
  std::cout << std::endl;
  test_accept_language_parsing();

  std::cout << "\n=== HTTP REQUEST TESTS COMPLETED ===" << std::endl;
  return 0;
}
