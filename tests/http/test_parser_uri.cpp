// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <string>

#include "http/RequestParser.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_happy_path_uri() {
  std::cout << "[Test] Happy path URI parsing..." << std::endl;
  RequestParser parser;

  // Feed "GET "
  assert(parser.feed('G') == STATE_METHOD);
  assert(parser.feed('E') == STATE_METHOD);
  assert(parser.feed('T') == STATE_METHOD);
  assert(parser.feed(' ') == STATE_URI);

  // Feed "/index.html "
  assert(parser.feed('/') == STATE_URI);
  assert(parser.feed('i') == STATE_URI);
  assert(parser.feed('n') == STATE_URI);
  assert(parser.feed('d') == STATE_URI);
  assert(parser.feed('e') == STATE_URI);
  assert(parser.feed('x') == STATE_URI);
  assert(parser.feed('.') == STATE_URI);
  assert(parser.feed('h') == STATE_URI);
  assert(parser.feed('t') == STATE_URI);
  assert(parser.feed('m') == STATE_URI);
  assert(parser.feed('l') == STATE_URI);

  // Delimiter space
  assert(parser.feed(' ') == STATE_VERSION);

  bool pass = (parser.get_request().get_uri() == "/index.html");
  print_result("test_happy_path_uri", pass);
}

void test_empty_uri_error() {
  std::cout << "[Test] Error handling for empty URI..." << std::endl;
  RequestParser parser;

  // Feed "GET  " (double space)
  assert(parser.feed('G') == STATE_METHOD);
  assert(parser.feed('E') == STATE_METHOD);
  assert(parser.feed('T') == STATE_METHOD);
  assert(parser.feed(' ') == STATE_URI);

  // Feed second space, which should trigger error because URI is empty
  assert(parser.feed(' ') == STATE_ERROR);

  print_result("test_empty_uri_error", true);
}

void test_control_character_in_uri() {
  std::cout << "[Test] Error handling for control characters in URI..."
            << std::endl;
  RequestParser parser;

  // Feed "GET /"
  assert(parser.feed('G') == STATE_METHOD);
  assert(parser.feed('E') == STATE_METHOD);
  assert(parser.feed('T') == STATE_METHOD);
  assert(parser.feed(' ') == STATE_URI);
  assert(parser.feed('/') == STATE_URI);

  // Feed control character SOH (ASCII 1)
  assert(parser.feed(1) == STATE_ERROR);

  print_result("test_control_character_in_uri", true);
}

void test_delete_character_in_uri() {
  std::cout << "[Test] Error handling for delete character (127) in URI..."
            << std::endl;
  RequestParser parser;

  // Feed "GET /"
  assert(parser.feed('G') == STATE_METHOD);
  assert(parser.feed('E') == STATE_METHOD);
  assert(parser.feed('T') == STATE_METHOD);
  assert(parser.feed(' ') == STATE_URI);
  assert(parser.feed('/') == STATE_URI);

  // Feed DEL (ASCII 127)
  assert(parser.feed(127) == STATE_ERROR);

  print_result("test_delete_character_in_uri", true);
}

int main() {
  std::cout << "=== STARTING REQUEST URI PARSER TESTS ===\n" << std::endl;

  test_happy_path_uri();
  std::cout << std::endl;
  test_empty_uri_error();
  std::cout << std::endl;
  test_control_character_in_uri();
  std::cout << std::endl;
  test_delete_character_in_uri();

  std::cout << "\n=== REQUEST URI PARSER TESTS COMPLETED ===" << std::endl;
  return 0;
}
