// Copyright 2026 serjimen vja-nie dlesieur
#include "http/RequestParser.hpp"
#include <iostream>
#include <cassert>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

void test_happy_path_request_line() {
    std::cout << "[Test] Happy path full Request-Line parsing..." << std::endl;
    RequestParser parser;

    // Feed "GET /index.html HTTP/1.1\r\n"
    const std::string request_line = "GET /index.html HTTP/1.1\r\n";
    for (size_t i = 0; i < request_line.length(); ++i) {
        parser.feed(request_line[i]);
    }

    bool pass = (parser.get_state() == STATE_HEADER_KEY);
    if (parser.get_request().get_method() != "GET") pass = false;
    if (parser.get_request().get_uri() != "/index.html") pass = false;
    if (parser.get_request().get_version() != "HTTP/1.1") pass = false;

    print_result("test_happy_path_request_line", pass);
}

void test_invalid_http_version() {
    std::cout << "[Test] Error handling for unsupported HTTP version (HTTP/1.0)..." << std::endl;
    RequestParser parser;

    const std::string request_line = "GET /index.html HTTP/1.0\r\n";
    bool hit_error = false;
    for (size_t i = 0; i < request_line.length(); ++i) {
        if (parser.feed(request_line[i]) == STATE_ERROR) {
            hit_error = true;
            break;
        }
    }

    print_result("test_invalid_http_version", hit_error);
}

void test_version_syntax_mismatch() {
    std::cout << "[Test] Error handling for version syntax mismatch (HTTP/1.1a)..." << std::endl;
    RequestParser parser;

    const std::string request_line = "GET /index.html HTTP/1.1a\r\n";
    bool hit_error = false;
    for (size_t i = 0; i < request_line.length(); ++i) {
        if (parser.feed(request_line[i]) == STATE_ERROR) {
            hit_error = true;
            break;
        }
    }

    print_result("test_version_syntax_mismatch", hit_error);
}

void test_missing_carriage_return() {
    std::cout << "[Test] Error handling for missing carriage return (\\r)..." << std::endl;
    RequestParser parser;

    const std::string request_line = "GET /index.html HTTP/1.1\n";
    bool hit_error = false;
    for (size_t i = 0; i < request_line.length(); ++i) {
        if (parser.feed(request_line[i]) == STATE_ERROR) {
            hit_error = true;
            break;
        }
    }

    print_result("test_missing_carriage_return", hit_error);
}

void test_invalid_char_after_carriage_return() {
    std::cout << "[Test] Error handling for invalid character after \\r..." << std::endl;
    RequestParser parser;

    const std::string request_line = "GET /index.html HTTP/1.1\r ";
    bool hit_error = false;
    for (size_t i = 0; i < request_line.length(); ++i) {
        if (parser.feed(request_line[i]) == STATE_ERROR) {
            hit_error = true;
            break;
        }
    }

    print_result("test_invalid_char_after_carriage_return", hit_error);
}

int main() {
    std::cout << "=== STARTING HTTP VERSION PARSER TESTS ===\n" << std::endl;

    test_happy_path_request_line();
    std::cout << std::endl;
    test_invalid_http_version();
    std::cout << std::endl;
    test_version_syntax_mismatch();
    std::cout << std::endl;
    test_missing_carriage_return();
    std::cout << std::endl;
    test_invalid_char_after_carriage_return();

    std::cout << "\n=== HTTP VERSION PARSER TESTS COMPLETED ===" << std::endl;
    return 0;
}
