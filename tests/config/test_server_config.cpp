// Copyright 2026 serjimen vja-nie dlesieur
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "config/ServerConfig.hpp"

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
  ServerConfig srv;

  bool pass = true;
  if (srv.get_port() != 8080) pass = false;
  if (srv.get_host() != "127.0.0.1") pass = false;
  if (!srv.get_server_names().empty()) pass = false;
  if (!srv.get_locations().empty()) pass = false;

  // Check inherited Context defaults
  if (!srv.getRoot().empty()) pass = false;

  print_result("test_initial_state", pass);
}

void test_setters_and_getters() {
  std::cout << "[Test] Verifying setters and getters..." << std::endl;
  ServerConfig srv;

  srv.set_port(443);
  srv.set_host("0.0.0.0");
  srv.add_server_name("example.com");
  srv.add_server_name("www.example.com");

  LocationConfig loc;
  loc.set_path("/");
  srv.add_location(loc);

  bool pass = true;
  if (srv.get_port() != 443) pass = false;
  if (srv.get_host() != "0.0.0.0") pass = false;

  const std::vector<std::string>& names = srv.get_server_names();
  if (names.size() != 2 || names[0] != "example.com" ||
      names[1] != "www.example.com")
    pass = false;

  const std::vector<LocationConfig>& locs = srv.get_locations();
  if (locs.size() != 1 || locs[0].get_path() != "/") pass = false;

  print_result("test_setters_and_getters", pass);
}

void test_canonical_form() {
  std::cout << "[Test] Verifying deep copy and assignment (including nested "
               "objects)..."
            << std::endl;
  ServerConfig srv1;
  srv1.set_port(80);
  srv1.add_server_name("orig");

  LocationConfig loc;
  loc.set_path("/api");
  srv1.add_location(loc);
  srv1.setRoot("/orig/root");

  // Copy constructor
  ServerConfig srv2(srv1);
  bool copy_pass =
      (srv2.get_port() == 80 && srv2.get_server_names().size() == 1 &&
       srv2.get_locations().size() == 1 &&
       srv2.get_locations()[0].get_path() == "/api" &&
       srv2.getRoot() == "/orig/root");

  // Assignment
  ServerConfig srv3;
  srv3 = srv1;
  bool assign_pass =
      (srv3.get_port() == 80 && srv3.get_server_names().size() == 1 &&
       srv3.get_locations().size() == 1 &&
       srv3.get_locations()[0].get_path() == "/api");

  // Verify deep copy of locations
  loc.set_path("/changed");
  srv1.set_locations(std::vector<LocationConfig>());
  srv1.add_location(loc);

  bool deep_pass = (srv2.get_locations()[0].get_path() == "/api");

  print_result("test_canonical_form", copy_pass && assign_pass && deep_pass);
}

void test_edge_cases() {
  std::cout << "[Test] Verifying edge cases (massive nesting)..." << std::endl;
  ServerConfig srv;

  // Massive number of locations
  for (int i = 0; i < 500; ++i) {
    LocationConfig loc;
    std::stringstream ss;
    ss << "/path-" << i;
    loc.set_path(ss.str());
    srv.add_location(loc);
  }

  bool massive_pass = (srv.get_locations().size() == 500 &&
                       srv.get_locations().back().get_path() == "/path-499");

  print_result("test_edge_cases", massive_pass);
}

void test_stress() {
  std::cout << "[Test] Stress test: 30,000 deep instantiations..." << std::endl;
  for (int i = 0; i < 30000; ++i) {
    ServerConfig srv;
    srv.set_port(i % 65535);

    for (int j = 0; j < 5; ++j) {
      LocationConfig loc;
      loc.set_path("/loc");
      srv.add_location(loc);
    }

    ServerConfig copy(srv);
    ServerConfig assign;
    assign = copy;

    if (assign.get_locations().size() != 5) {
      print_result("test_stress", false);
      return;
    }
  }
  print_result("test_stress", true);
}

void test_longest_prefix_match() {
  std::cout << "[Test] Verifying Longest Prefix Match routing algorithm..."
            << std::endl;
  ServerConfig srv;

  // Setup locations
  LocationConfig loc_root;
  loc_root.set_path("/");
  srv.add_location(loc_root);

  LocationConfig loc_api;
  loc_api.set_path("/api");
  srv.add_location(loc_api);

  LocationConfig loc_users;
  loc_users.set_path("/api/users");
  srv.add_location(loc_users);

  LocationConfig loc_images;
  loc_images.set_path("/images/");
  srv.add_location(loc_images);

  bool pass = true;

  // 1. Exact Match
  const LocationConfig* res = srv.find_location("/api");
  if (!res || res->get_path() != "/api") pass = false;

  // 2. Prefix Match
  res = srv.find_location("/api/docs");
  if (!res || res->get_path() != "/api") pass = false;

  // 3. Longest Prefix Wins
  res = srv.find_location("/api/users/profile");
  if (!res || res->get_path() != "/api/users") pass = false;

  // 4. Strict Boundary Check (Failure Case)
  // "/api_v2" starts with "/api" but is not a subpath.
  // Should match root "/" or NULL.
  res = srv.find_location("/api_v2");
  if (!res || res->get_path() != "/") pass = false;

  // 5. Trailing Slash Handling
  res = srv.find_location("/images/logo.png");
  if (!res || res->get_path() != "/images/") pass = false;

  // 6. Root Fallback
  res = srv.find_location("/something_else");
  if (!res || res->get_path() != "/") pass = false;

  print_result("test_longest_prefix_match", pass);
}

int main() {
  std::cout << "=== STARTING SERVER CONFIG TESTS ===\n" << std::endl;

  test_initial_state();
  std::cout << std::endl;
  test_setters_and_getters();
  std::cout << std::endl;
  test_canonical_form();
  std::cout << std::endl;
  test_longest_prefix_match();
  std::cout << std::endl;
  test_edge_cases();
  std::cout << std::endl;
  test_stress();

  std::cout << "\n=== SERVER CONFIG TESTS COMPLETED ===" << std::endl;
  return 0;
}
