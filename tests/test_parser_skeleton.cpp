#include "http/RequestParser.hpp"
#include <iostream>
#include <cassert>

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

void test_parser_initial_state() {
    std::cout << "[Test] Verifying RequestParser initial state..." << std::endl;
    RequestParser parser;

    bool pass = (parser.get_state() == STATE_START);
    print_result("test_parser_initial_state", pass);
}

void test_parser_feed() {
    std::cout << "[Test] Verifying feed method compilation and baseline execution..." << std::endl;
    RequestParser parser;

    // Feed a character, it should return the current state (which remains STATE_START for now)
    e_parser_state state = parser.feed('G');
    bool pass = (state == STATE_START);
    print_result("test_parser_feed", pass);
}

void test_parser_reset() {
    std::cout << "[Test] Verifying reset method..." << std::endl;
    RequestParser parser;

    // Even though state doesn't change yet, we make sure calling reset works
    parser.reset();
    bool pass = (parser.get_state() == STATE_START);
    print_result("test_parser_reset", pass);
}

int main() {
    std::cout << "=== STARTING REQUEST PARSER SKELETON TESTS ===\n" << std::endl;

    test_parser_initial_state();
    std::cout << std::endl;
    test_parser_feed();
    std::cout << std::endl;
    test_parser_reset();

    std::cout << "\n=== REQUEST PARSER SKELETON TESTS COMPLETED ===" << std::endl;
    return 0;
}
