// Copyright 2026 serjimen vja-nie dlesieur
#include "http/RequestParser.hpp"
#include <iostream>
#include <cassert>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

static e_parser_state feed_string(RequestParser& parser, const std::string& str) {
    e_parser_state state = parser.get_state();
    for (size_t i = 0; i < str.length(); ++i) {
        state = parser.feed(str[i]);
        if (state == STATE_ERROR) {
            break;
        }
    }
    return state;
}

void test_get_request_no_body() {
    std::cout << "[Test] GET request with no Content-Length header..." << std::endl;
    RequestParser parser;

    feed_string(parser, "GET /index.html HTTP/1.1\r\n");
    feed_string(parser, "Host: localhost\r\n");
    e_parser_state state = feed_string(parser, "\r\n");

    print_result("test_get_request_no_body", state == STATE_COMPLETE);
}

void test_post_request_zero_content_length() {
    std::cout << "[Test] POST request with Content-Length: 0..." << std::endl;
    RequestParser parser;

    feed_string(parser, "POST /submit HTTP/1.1\r\n");
    feed_string(parser, "Host: localhost\r\n");
    feed_string(parser, "Content-Length: 0\r\n");
    e_parser_state state = feed_string(parser, "\r\n");

    print_result("test_post_request_zero_content_length", state == STATE_COMPLETE);
}

void test_post_request_with_body() {
    std::cout << "[Test] POST request with Content-Length and body..." << std::endl;
    RequestParser parser;

    feed_string(parser, "POST /submit HTTP/1.1\r\n");
    feed_string(parser, "Host: localhost\r\n");
    feed_string(parser, "Content-Length: 5\r\n");
    feed_string(parser, "\r\n");

    assert(parser.get_state() == STATE_BODY);

    e_parser_state state = feed_string(parser, "hello");
    bool check_state = (state == STATE_COMPLETE);
    bool check_body = (parser.get_request().get_body() == "hello");

    print_result("test_post_request_with_body", check_state && check_body);
}

void test_post_request_invalid_content_length() {
    std::cout << "[Test] POST request with non-numeric Content-Length..." << std::endl;
    RequestParser parser;

    feed_string(parser, "POST /submit HTTP/1.1\r\n");
    feed_string(parser, "Host: localhost\r\n");
    feed_string(parser, "Content-Length: 12abc\r\n");
    e_parser_state state = feed_string(parser, "\r\n");

    print_result("test_post_request_invalid_content_length", state == STATE_ERROR);
}

int main() {
    std::cout << "=== STARTING HTTP BODY PARSER TESTS ===\n" << std::endl;

    test_get_request_no_body();
    std::cout << std::endl;
    test_post_request_zero_content_length();
    std::cout << std::endl;
    test_post_request_with_body();
    std::cout << std::endl;
    test_post_request_invalid_content_length();

    std::cout << "\n=== HTTP BODY PARSER TESTS COMPLETED ===" << std::endl;
    return 0;
}
