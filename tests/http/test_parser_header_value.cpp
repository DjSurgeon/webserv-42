// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <map>
#include <string>

#include "http/RequestParser.hpp"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

// Helper to push a string of characters into the parser
static e_parser_state feed_string(RequestParser* parser,
                                  const std::string& str) {
  if (!parser) {
    return STATE_ERROR;
  }
  e_parser_state state = parser->get_state();
  for (size_t i = 0; i < str.length(); ++i) {
    state = parser->feed(str[i]);
    if (state == STATE_ERROR) {
      break;
    }
  }
  return state;
}

// Helper to retrieve header values from HttpRequest
static std::string get_header_val(const HttpRequest& req,
                                  const std::string& key) {
  const std::map<std::string, std::string>& headers = req.get_headers();
  std::map<std::string, std::string>::const_iterator it = headers.find(key);
  if (it != headers.end()) {
    return it->second;
  }
  return "";
}

void test_happy_path_header_value() {
  std::cout << "[Test] Parsing valid header key and value..." << std::endl;
  RequestParser parser;

  // Transition through Request-Line
  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  // Feed key "Host"
  feed_string(&parser, "Host:");
  assert(parser.get_state() == STATE_HEADER_VALUE);

  // Feed value with spaces and tabs on edges: "  \t  localhost:8080   \t "
  feed_string(&parser, "  \t  localhost:8080   \t \r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  const HttpRequest& req = parser.get_request();
  bool check = (get_header_val(req, "host") == "localhost:8080");

  print_result("test_happy_path_header_value", check);
}

void test_multiple_headers_cycling() {
  std::cout << "[Test] Cycling through multiple headers..." << std::endl;
  RequestParser parser;

  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  feed_string(&parser, "Host: localhost\r\n");
  feed_string(&parser, "Content-Length: 42\r\n");
  feed_string(&parser, "Connection: close\r\n");
  feed_string(&parser, "\r\n");  // End headers block

  assert(parser.get_state() == STATE_BODY);

  const HttpRequest& req = parser.get_request();
  bool check1 = (get_header_val(req, "host") == "localhost");
  bool check2 = (get_header_val(req, "content-length") == "42");
  bool check3 = (get_header_val(req, "connection") == "close");

  print_result("test_multiple_headers_cycling", check1 && check2 && check3);
}

void test_empty_header_value() {
  std::cout << "[Test] Parsing empty header value..." << std::endl;
  RequestParser parser;

  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  feed_string(&parser, "X-Empty-Header:   \t   \r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  const HttpRequest& req = parser.get_request();
  bool check = (get_header_val(req, "x-empty-header") == "");

  print_result("test_empty_header_value", check);
}

void test_invalid_header_value_control_char() {
  std::cout << "[Test] Rejection of control characters in value..."
            << std::endl;
  RequestParser parser;

  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  feed_string(&parser, "Host: ");

  // ASCII 7 is a bell (control char), which is forbidden in header value
  e_parser_state state = parser.feed(7);
  print_result("test_invalid_header_value_control_char", state == STATE_ERROR);
}

int main() {
  std::cout << "=== STARTING HTTP HEADER VALUE PARSER TESTS ===\n" << std::endl;

  test_happy_path_header_value();
  std::cout << std::endl;
  test_multiple_headers_cycling();
  std::cout << std::endl;
  test_empty_header_value();
  std::cout << std::endl;
  test_invalid_header_value_control_char();

  std::cout << "\n=== HTTP HEADER VALUE PARSER TESTS COMPLETED ==="
            << std::endl;
  return 0;
}
