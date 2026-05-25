// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
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

void test_happy_path_header_key() {
  std::cout << "[Test] Parsing valid header key..." << std::endl;
  RequestParser parser;

  // First transition through the Request-Line
  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  // Feed a standard header key: "Host"
  e_parser_state state1 = feed_string(&parser, "Host");
  bool check1 = (state1 == STATE_HEADER_KEY);

  // Feed the colon separator
  e_parser_state state2 = parser.feed(':');
  bool check2 = (state2 == STATE_HEADER_VALUE);

  print_result("test_happy_path_header_key", check1 && check2);
}

void test_double_crlf_transition() {
  std::cout << "[Test] Detecting double CRLF transition to body..."
            << std::endl;
  RequestParser parser;

  // Transition through Request-Line and supply Content-Length header
  feed_string(&parser, "POST /submit HTTP/1.1\r\n");
  feed_string(&parser, "Content-Length: 5\r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  // Encounter immediate \r\n (double CRLF since line started)
  e_parser_state state1 = parser.feed('\r');
  bool check1 =
      (state1 ==
       STATE_HEADER_KEY);  // expecting newline flag set, state unchanged yet

  e_parser_state state2 = parser.feed('\n');
  bool check2 = (state2 == STATE_BODY);  // Transitions to STATE_BODY!

  print_result("test_double_crlf_transition", check1 && check2);
}

void test_empty_header_key_error() {
  std::cout << "[Test] Error on colon separator at start of line..."
            << std::endl;
  RequestParser parser;

  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  // Immediate colon ':' at start of line (empty key name)
  e_parser_state state = parser.feed(':');
  print_result("test_empty_header_key_error", state == STATE_ERROR);
}

void test_invalid_header_key_char_error() {
  std::cout << "[Test] Error on space in header key..." << std::endl;
  RequestParser parser;

  feed_string(&parser, "GET /index.html HTTP/1.1\r\n");
  assert(parser.get_state() == STATE_HEADER_KEY);

  // A space character ' ' is illegal inside header keys (RFC 7230 3.2.4:
  // field-name = token, no space allowed)
  e_parser_state state = feed_string(&parser, "Host ");
  print_result("test_invalid_header_key_char_error", state == STATE_ERROR);
}

int main() {
  std::cout << "=== STARTING HTTP HEADER KEY PARSER TESTS ===\n" << std::endl;

  test_happy_path_header_key();
  std::cout << std::endl;
  test_double_crlf_transition();
  std::cout << std::endl;
  test_empty_header_key_error();
  std::cout << std::endl;
  test_invalid_header_key_char_error();

  std::cout << "\n=== HTTP HEADER KEY PARSER TESTS COMPLETED ===" << std::endl;
  return 0;
}
