// Copyright 2026 serjimen vja-nie dlesieur
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
  if (!loc.get_path().empty()) pass = false;
  if (!loc.get_allowed_methods().empty()) pass = false;
  if (!loc.get_cgi_path().empty()) pass = false;
  if (!loc.get_redirect().empty()) pass = false;

  // Check inherited Context defaults
  if (!loc.get_root().empty()) pass = false;
  if (loc.get_client_max_body_size() != 1048576) pass = false;

  print_result("test_initial_state", pass);
}

void test_setters_and_getters() {
  std::cout << "[Test] Verifying setters and getters..." << std::endl;
  LocationConfig loc;

  loc.set_path("/api");
  loc.add_allowed_method("GET");
  loc.add_allowed_method("POST");
  loc.set_cgi_path("/usr/bin/php-cgi");
  loc.set_redirect("http://example.com");

  // Inherited
  loc.set_root("/var/www/api");
  loc.set_autoindex(true);

  bool pass = true;
  if (loc.get_path() != "/api") pass = false;

  const std::vector<std::string>& methods = loc.get_allowed_methods();
  if (methods.size() != 2 || methods[0] != "GET" || methods[1] != "POST")
    pass = false;

  if (loc.get_cgi_path() != "/usr/bin/php-cgi") pass = false;
  if (loc.get_redirect() != "http://example.com") pass = false;
  if (loc.get_root() != "/var/www/api") pass = false;
  if (loc.get_autoindex() != true) pass = false;

  print_result("test_setters_and_getters", pass);
}

void test_canonical_form() {
  std::cout
      << "[Test] Verifying deep copy and assignment (including Context)..."
      << std::endl;
  LocationConfig loc1;
  loc1.set_path("/old");
  loc1.add_allowed_method("DELETE");
  loc1.set_root("/old/root");

  // Copy constructor
  LocationConfig loc2(loc1);
  bool copy_pass =
      (loc2.get_path() == "/old" && loc2.get_allowed_methods().size() == 1 &&
       loc2.get_root() == "/old/root");

  // Assignment
  LocationConfig loc3;
  loc3 = loc1;
  bool assign_pass =
      (loc3.get_path() == "/old" && loc3.get_allowed_methods().size() == 1 &&
       loc3.get_root() == "/old/root");

  // Self assignment check
  loc3 = loc3;
  bool self_assign_pass = (loc3.get_path() == "/old");

  // Verify deep copy (modify original)
  loc1.set_path("/new");
  loc1.add_allowed_method("PUT");
  bool deep_pass =
      (loc2.get_path() == "/old" && loc2.get_allowed_methods().size() == 1);

  print_result("test_canonical_form",
               copy_pass && assign_pass && self_assign_pass && deep_pass);
}

void test_edge_cases() {
  std::cout
      << "[Test] Verifying edge cases (empty strings and massive vectors)..."
      << std::endl;
  LocationConfig loc;

  // Empty path and methods
  loc.set_path("");
  loc.set_allowed_methods(std::vector<std::string>());
  bool empty_pass =
      (loc.get_path().empty() && loc.get_allowed_methods().empty());

  // Massive vector of methods
  for (int i = 0; i < 1000; ++i) {
    std::stringstream ss;
    ss << "METHOD-" << i;
    loc.add_allowed_method(ss.str());
  }
  bool massive_pass = (loc.get_allowed_methods().size() == 1000 &&
                       loc.get_allowed_methods().back() == "METHOD-999");

  print_result("test_edge_cases", empty_pass && massive_pass);
}

void test_stress() {
  std::cout << "[Test] Stress test: 50,000 instantiations/copies..."
            << std::endl;
  for (int i = 0; i < 50000; ++i) {
    LocationConfig loc;
    loc.set_path("/stress");
    loc.add_allowed_method("GET");
    loc.add_error_page(404, "/404.html");

    LocationConfig copy(loc);
    LocationConfig assign;
    assign = copy;

    if (assign.get_path() != "/stress") {
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
