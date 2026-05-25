// Copyright 2026 serjimen vja-nie dlesieur
#include "http/HttpRequest.hpp"
#include <iostream>
#include <cassert>
#include <map>

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void print_result(const std::string& test_name, bool success) {
    std::cout << test_name << ": " << (success ? std::string(GREEN) + "PASSED" : std::string(RED) + "FAILED") << RESET << std::endl;
}

void test_single_cookie() {
    std::cout << "[Test] Verifying single cookie parsing..." << std::endl;
    HttpRequest req;
    
    // Simulate parser adding a Cookie header
    req.add_header("Cookie", "user=serjimen");
    
    const std::map<std::string, std::string>& cookies = req.get_cookies();
    bool pass = false;
    
    std::map<std::string, std::string>::const_iterator it = cookies.find("user");
    if (it != cookies.end() && it->second == "serjimen") {
        pass = true;
    }
    
    print_result("test_single_cookie", pass);
}

void test_multiple_cookies() {
    std::cout << "[Test] Verifying multiple cookies separated by ;..." << std::endl;
    HttpRequest req;
    
    req.add_header("Cookie", "user=serjimen; theme=dark; lang=es");
    
    const std::map<std::string, std::string>& cookies = req.get_cookies();
    bool pass = true;
    
    if (cookies.size() != 3) pass = false;
    
    std::map<std::string, std::string>::const_iterator it = cookies.find("user");
    if (it == cookies.end() || it->second != "serjimen") pass = false;
    
    it = cookies.find("theme");
    if (it == cookies.end() || it->second != "dark") pass = false;

    it = cookies.find("lang");
    if (it == cookies.end() || it->second != "es") pass = false;

    print_result("test_multiple_cookies", pass);
}

void test_cookies_with_extra_spaces() {
    std::cout << "[Test] Verifying resistance against extra spaces..." << std::endl;
    HttpRequest req;
    
    req.add_header("Cookie", "   user=serjimen   ;   theme=dark   ;   ");
    
    const std::map<std::string, std::string>& cookies = req.get_cookies();
    bool pass = true;
    
    // We expect 2 valid cookies
    if (cookies.size() != 2) pass = false;
    
    std::map<std::string, std::string>::const_iterator it = cookies.find("user");
    if (it == cookies.end() || it->second != "serjimen") pass = false;
    
    it = cookies.find("theme");
    if (it == cookies.end() || it->second != "dark") pass = false;

    print_result("test_cookies_with_extra_spaces", pass);
}

int main() {
    std::cout << "=== STARTING HTTP COOKIES TESTS ===\n" << std::endl;

    test_single_cookie();
    std::cout << std::endl;
    test_multiple_cookies();
    std::cout << std::endl;
    test_cookies_with_extra_spaces();

    std::cout << "\n=== HTTP COOKIES TESTS COMPLETED ===" << std::endl;
    return 0;
}
