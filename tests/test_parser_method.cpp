#include "http/RequestParser.hpp"
#include <iostream>
#include <cassert>

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

void test_happy_path_get() {
    std::cout << "[Test] Happy path GET request method parsing..." << std::endl;
    RequestParser parser;

    assert(parser.get_state() == STATE_START);

    // Feed 'G'
    assert(parser.feed('G') == STATE_METHOD);
    // Feed 'E'
    assert(parser.feed('E') == STATE_METHOD);
    // Feed 'T'
    assert(parser.feed('T') == STATE_METHOD);
    // Feed space ' '
    assert(parser.feed(' ') == STATE_URI);

    bool pass = (parser.get_request().get_method() == "GET");
    print_result("test_happy_path_get", pass);
}

void test_skip_leading_whitespace_and_newlines() {
    std::cout << "[Test] Skipping leading whitespaces, carriage returns, and newlines..." << std::endl;
    RequestParser parser;

    // Feed leading chars
    assert(parser.feed(' ') == STATE_START);
    assert(parser.feed('\r') == STATE_START);
    assert(parser.feed('\n') == STATE_START);
    assert(parser.feed(' ') == STATE_START);

    // Feed "POST "
    assert(parser.feed('P') == STATE_METHOD);
    assert(parser.feed('O') == STATE_METHOD);
    assert(parser.feed('S') == STATE_METHOD);
    assert(parser.feed('T') == STATE_METHOD);
    assert(parser.feed(' ') == STATE_URI);

    bool pass = (parser.get_request().get_method() == "POST");
    print_result("test_skip_leading_whitespace_and_newlines", pass);
}

void test_invalid_character_in_method() {
    std::cout << "[Test] Error handling for invalid characters in method..." << std::endl;
    RequestParser parser;

    assert(parser.feed('G') == STATE_METHOD);
    assert(parser.feed('E') == STATE_METHOD);
    // Feed invalid char '1'
    assert(parser.feed('1') == STATE_ERROR);

    // Verify it stays in error state
    assert(parser.feed('T') == STATE_ERROR);

    print_result("test_invalid_character_in_method", true);
}

void test_invalid_start_character() {
    std::cout << "[Test] Error handling for invalid starting characters..." << std::endl;
    RequestParser parser;

    // Feed invalid starting char
    assert(parser.feed('/') == STATE_ERROR);

    print_result("test_invalid_start_character", true);
}

int main() {
    std::cout << "=== STARTING REQUEST METHOD PARSER TESTS ===\n" << std::endl;

    test_happy_path_get();
    std::cout << std::endl;
    test_skip_leading_whitespace_and_newlines();
    std::cout << std::endl;
    test_invalid_character_in_method();
    std::cout << std::endl;
    test_invalid_start_character();

    std::cout << "\n=== REQUEST METHOD PARSER TESTS COMPLETED ===" << std::endl;
    return 0;
}
