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

    // Feed a space (ignored leading character), it should remain in STATE_START
    e_parser_state state1 = parser.feed(' ');
    // Feed 'G' (alphabetic), it should transition to STATE_METHOD
    e_parser_state state2 = parser.feed('G');
    
    bool pass = (state1 == STATE_START && state2 == STATE_METHOD);
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
