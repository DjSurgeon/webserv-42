// Copyright 2026 serjimen vja-nie dlesieur
#include "config/ConfigParser.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

// Global pass/fail tracking
static bool g_all_passed = true;

static void print_result(const std::string& test_name, bool success) {
    if (!success) g_all_passed = false;
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

void test_file_loading() {
    std::cout << "[Test] Verifying file loading and line count..." << std::endl;
    try {
        // tests/assets/test_basic.conf has 5 lines, but 1 is a full comment.
        // New logic skips full comments. Expect 4 lines.
        ConfigParser parser("tests/assets/test_basic.conf");
        
        bool count_pass = (parser.get_raw_lines().size() == 4);
        if (!count_pass) {
            std::cerr << "Expected 4 lines, got " << parser.get_raw_lines().size() << std::endl;
        }
        
        print_result("test_file_loading", count_pass);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        print_result("test_file_loading", false);
    }
}

void test_invalid_file() {
    std::cout << "[Test] Verifying error on missing file..." << std::endl;
    bool exception_caught = false;
    try {
        ConfigParser parser("non_existent_file_404.conf");
    } catch (const std::runtime_error& e) {
        exception_caught = true;
    }

    print_result("test_invalid_file", exception_caught);
}

void test_canonical_form() {
    std::cout << "[Test] Verifying canonical form for ConfigParser..." << std::endl;
    try {
        ConfigParser parser1("tests/assets/test_basic.conf");
        
        // Copy constructor
        ConfigParser parser2(parser1);
        bool copy_pass = (parser2.get_raw_lines().size() == 4);
        
        // Assignment operator
        ConfigParser parser3;
        parser3 = parser1;
        bool assign_pass = (parser3.get_raw_lines().size() == 4);
        
        print_result("test_canonical_form", copy_pass && assign_pass);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        print_result("test_canonical_form", false);
    }
}

void test_preprocessing_edge_cases() {
    std::cout << "[Test] Verifying preprocessing edge cases (whitespace and comments)..." << std::endl;
    try {
        ConfigParser parser("tests/assets/test_edge_cases.conf");
        const std::vector<std::string>& lines = parser.get_raw_lines();

        // Recounting test_edge_cases.conf with new logic:
        // Meaningful lines: 
        // 1. "server { # inline comment after directive" -> "server {"
        // 2. "    listen 80;    " -> "listen 80;"
        // 3. "    server_name localhost;    # another inline comment" -> "server_name localhost;"
        // 4. "\t \t location / { \t # tabbed line with comment" -> "location / {"
        // 5. "\t \t \t root /var/www;" -> "root /var/www;"
        // 6. "\t \t }" -> "}"
        // 7. "}" -> "}"
        // Total: 7 lines.
        
        bool count_pass = (lines.size() == 7);
        if (!count_pass) {
            std::cerr << "Expected 7 lines, got " << lines.size() << std::endl;
            for (size_t i = 0; i < lines.size(); ++i) {
                std::cerr << "  Line " << i << ": [" << lines[i] << "]" << std::endl;
            }
        }

        bool content_pass = true;
        if (count_pass) {
            if (lines[0] != "server {") content_pass = false;
            if (lines[1] != "listen 80;") content_pass = false;
            if (lines[2] != "server_name localhost;") content_pass = false;
            // Check that comments and extra space are gone
            if (lines[3] != "location / {") content_pass = false;
            if (lines[4] != "root /var/www;") content_pass = false;
            if (lines[5] != "}") content_pass = false;
            if (lines[6] != "}") content_pass = false;
        }

        print_result("test_preprocessing_edge_cases", count_pass && content_pass);
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        print_result("test_preprocessing_edge_cases", false);
    }
}

int main() {
    std::cout << "=== STARTING CONFIG PARSER TESTS ===\n" << std::endl;

    test_file_loading();
    std::cout << std::endl;
    test_invalid_file();
    std::cout << std::endl;
    test_canonical_form();
    std::cout << std::endl;
    test_preprocessing_edge_cases();

    std::cout << "\n=== CONFIG PARSER TESTS COMPLETED ===" << std::endl;
    return g_all_passed ? 0 : 1;
}
