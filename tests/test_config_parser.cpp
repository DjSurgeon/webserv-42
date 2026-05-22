// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

// Global pass/fail tracking
static bool g_all_passed = true;

static void print_result(const std::string& test_name, bool success) {
  if (!success) g_all_passed = false;
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_file_loading() {
  std::cout << "[Test] Verifying file loading and line count..." << std::endl;
  try {
    ConfigParser parser("tests/assets/test_basic.conf");
    bool count_pass = (parser.get_raw_lines().size() == 4);
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

void test_preprocessing_edge_cases() {
  std::cout << "[Test] Verifying preprocessing edge cases..." << std::endl;
  try {
    ConfigParser parser("tests/assets/test_edge_cases.conf");
    const std::vector<std::string>& lines = parser.get_raw_lines();
    bool count_pass = (lines.size() == 7);
    print_result("test_preprocessing_edge_cases", count_pass);
  } catch (const std::exception& e) {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
    print_result("test_preprocessing_edge_cases", false);
  }
}

void test_block_parsing_valid() {
  std::cout << "[Test] Verifying complex but valid block parsing..."
            << std::endl;
  try {
    ConfigParser parser("tests/assets/test_complex_valid.conf");
    const std::vector<ServerConfig>& servers = parser.get_servers();

    // test_complex_valid.conf: "server { location / { } }
    // server{location/api{}}"
    bool pass = (servers.size() == 2);
    if (pass) {
      if (servers[0].get_locations().size() != 1) pass = false;
      if (servers[0].get_locations()[0].get_path() != "/") pass = false;
      if (servers[1].get_locations().size() != 1) pass = false;
      if (servers[1].get_locations()[0].get_path() != "/api") pass = false;
    }

    print_result("test_block_parsing_valid", pass);
  } catch (const std::exception& e) {
    std::cerr << "Unexpected exception: " << e.what() << std::endl;
    print_result("test_block_parsing_valid", false);
  }
}

struct TestCase {
  std::string file;
  std::string description;
};

void test_block_parsing_errors() {
  std::cout << "[Test] Verifying syntax error detection..." << std::endl;

  std::vector<TestCase> cases;

  TestCase c1 = {"tests/assets/test_invalid_missing_brace.conf",
                 "Missing brace"};
  TestCase c2 = {"tests/assets/test_invalid_no_brace_after_server.conf",
                 "No brace after server"};
  TestCase c3 = {"tests/assets/test_invalid_root_directive.conf",
                 "Directive at root level"};
  TestCase c4 = {"tests/assets/test_invalid_missing_location_path.conf",
                 "Missing location path"};

  cases.push_back(c1);
  cases.push_back(c2);
  cases.push_back(c3);
  cases.push_back(c4);

  bool all_caught = true;
  for (size_t i = 0; i < cases.size(); ++i) {
    try {
      ConfigParser parser(cases[i].file);
      std::cerr << "Fail: Expected exception for: " << cases[i].description
                << std::endl;
      all_caught = false;
    } catch (const std::runtime_error& e) {
      // std::cout << "  (Caught expected error: " << e.what() << ")" <<
      // std::endl;
    }
  }

  print_result("test_block_parsing_errors", all_caught);
}

void test_stress_parsing() {
  std::cout << "[Test] Stress testing with 1000 server blocks..." << std::endl;
  const std::string filename = "tests/assets/test_stress.conf";
  std::ofstream out(filename.c_str());
  for (int i = 0; i < 1000; ++i) {
    out << "server { location /s" << i << " { } }\n";
  }
  out.close();

  try {
    ConfigParser parser(filename);
    bool pass = (parser.get_servers().size() == 1000);
    print_result("test_stress_parsing", pass);
  } catch (const std::exception& e) {
    std::cerr << "Stress test failed: " << e.what() << std::endl;
    print_result("test_stress_parsing", false);
  }
  std::remove(filename.c_str());
}

int main() {
  std::cout << "=== STARTING CONFIG PARSER STRUCTURAL TESTS ===\n" << std::endl;

  test_file_loading();
  test_invalid_file();
  test_preprocessing_edge_cases();
  std::cout << std::endl;
  test_block_parsing_valid();
  test_block_parsing_errors();
  std::cout << std::endl;
  test_stress_parsing();

  std::cout << "\n=== CONFIG PARSER TESTS COMPLETED ===" << std::endl;
  return g_all_passed ? 0 : 1;
}
