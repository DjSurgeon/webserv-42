// Copyright 2026 serjimen vja-nie dlesieur
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

static void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_initial_state() {
  std::cout << "[Test] Verifying default constructor state..." << std::endl;
  LocationConfig loc;

  bool pass = true;
  if (!loc.getPath().empty()) pass = false;
  if (!loc.getAllowedMethods().empty()) pass = false;
  if (!loc.getCgiPath().empty()) pass = false;
  if (!loc.getCgiExtensions().empty()) pass = false;
  if (!loc.getRedirect().empty()) pass = false;

  // Inherited Context defaults
  if (!loc.getRoot().empty()) pass = false;
  if (loc.getClientMaxBodySize() != 1048576) pass = false;

  print_result("test_initial_state", pass);
}

void test_setters_and_getters() {
  std::cout << "[Test] Verifying setters and getters..." << std::endl;
  LocationConfig loc;

  loc.setPath("/api");
  loc.addAllowedMethod("GET");
  loc.addAllowedMethod("POST");
  loc.setCgiPath("/usr/bin/php-cgi");
  loc.setRedirect("http://example.com");

  // Inherited
  loc.setRoot("/var/www/api");
  loc.setAutoindex(true);

  bool pass = true;
  if (loc.getPath() != "/api") pass = false;

  const std::vector<std::string>& methods = loc.getAllowedMethods();
  if (methods.size() != 2 || methods[0] != "GET" || methods[1] != "POST")
    pass = false;

  if (loc.getCgiPath() != "/usr/bin/php-cgi") pass = false;
  if (loc.getRedirect() != "http://example.com") pass = false;
  if (loc.getRoot() != "/var/www/api") pass = false;
  if (loc.getAutoindex() != true) pass = false;

  print_result("test_setters_and_getters", pass);
}

void test_canonical_form() {
  std::cout
      << "[Test] Verifying deep copy and assignment (including Context)..."
      << std::endl;
  LocationConfig loc1;
  loc1.setPath("/old");
  loc1.addAllowedMethod("DELETE");
  loc1.setRoot("/old/root");

  // Copy constructor
  LocationConfig loc2(loc1);
  bool copy_pass =
      (loc2.getPath() == "/old" && loc2.getAllowedMethods().size() == 1 &&
       loc2.getRoot() == "/old/root");

  // Assignment
  LocationConfig loc3;
  loc3 = loc1;
  bool assign_pass =
      (loc3.getPath() == "/old" && loc3.getAllowedMethods().size() == 1 &&
       loc3.getRoot() == "/old/root");

  // Self assignment check
  loc3 = loc3;
  bool self_assign_pass = (loc3.getPath() == "/old");

  // Verify deep copy (modify original)
  loc1.setPath("/new");
  loc1.addAllowedMethod("PUT");
  bool deep_pass =
      (loc2.getPath() == "/old" && loc2.getAllowedMethods().size() == 1);

  print_result("test_canonical_form",
               copy_pass && assign_pass && self_assign_pass && deep_pass);
}

void test_edge_cases() {
  std::cout
      << "[Test] Verifying edge cases (empty strings and massive vectors)..."
      << std::endl;
  LocationConfig loc;

  // Empty path and methods
  loc.setPath("");
  loc.setAllowedMethods(std::vector<std::string>());
  bool reset_pass =
      (loc.getPath().empty() && loc.getAllowedMethods().empty());

  // Massive vector of methods
  for (int i = 0; i < 1000; ++i) {
    std::stringstream ss;
    ss << "METHOD-" << i;
    loc.addAllowedMethod(ss.str());
  }
  bool massive_pass = (loc.getAllowedMethods().size() == 1000 &&
                       loc.getAllowedMethods().back() == "METHOD-999");

  print_result("test_edge_cases", reset_pass && massive_pass);
}

void test_stress() {
  std::cout << "[Test] Stress test: 50,000 instantiations/copies..."
            << std::endl;
  for (int i = 0; i < 50000; ++i) {
    LocationConfig loc;
    loc.setPath("/stress");
    loc.addAllowedMethod("GET");
    loc.addErrorPage(404, "/404.html");

    LocationConfig copy(loc);
    LocationConfig assign;
    assign = copy;

    if (assign.getPath() != "/stress") {
      print_result("test_stress", false);
      return;
    }
  }
  print_result("test_stress", true);
}

int main() {
  std::cout << "=== STARTING LOCATION CONFIG TESTS ===\n" << std::endl;

  test_initial_state();
  std::cout << std::endl;
  test_setters_and_getters();
  std::cout << std::endl;
  test_canonical_form();
  std::cout << std::endl;
  test_edge_cases();
  std::cout << std::endl;
  test_stress();

  std::cout << "\n=== LOCATION CONFIG TESTS COMPLETED ===" << std::endl;
  return 0;
}
