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
  if (srv.getPort() != 8080) pass = false;
  if (srv.getHost() != "127.0.0.1") pass = false;
  if (!srv.getServerNames().empty()) pass = false;
  if (!srv.getLocations().empty()) pass = false;

  // Check inherited Context defaults
  if (!srv.getRoot().empty()) pass = false;

  print_result("test_initial_state", pass);
}

void test_setters_and_getters() {
  std::cout << "[Test] Verifying setters and getters..." << std::endl;
  ServerConfig srv;

  srv.setPort(443);
  srv.setHost("0.0.0.0");
  srv.addServerName("example.com");
  srv.addServerName("www.example.com");

  LocationConfig loc;
  loc.setPath("/");
  srv.addLocation(loc);

  bool pass = true;
  if (srv.getPort() != 443) pass = false;
  if (srv.getHost() != "0.0.0.0") pass = false;

  const std::vector<std::string>& names = srv.getServerNames();
  if (names.size() != 2 || names[0] != "example.com" ||
      names[1] != "www.example.com")
    pass = false;

  const std::vector<LocationConfig>& locs = srv.getLocations();
  if (locs.size() != 1 || locs[0].getPath() != "/") pass = false;

  print_result("test_setters_and_getters", pass);
}

void test_canonical_form() {
  std::cout << "[Test] Verifying deep copy and assignment (including nested "
               "objects)..."
            << std::endl;
  ServerConfig srv1;
  srv1.setPort(80);
  srv1.addServerName("orig");

  LocationConfig loc;
  loc.setPath("/api");
  srv1.addLocation(loc);
  srv1.setRoot("/orig/root");

  // Copy constructor
  ServerConfig srv2(srv1);
  bool copy_pass =
      (srv2.getPort() == 80 && srv2.getServerNames().size() == 1 &&
       srv2.getLocations().size() == 1 &&
       srv2.getLocations()[0].getPath() == "/api" &&
       srv2.getRoot() == "/orig/root");

  // Assignment
  ServerConfig srv3;
  srv3 = srv1;
  bool assign_pass =
      (srv3.getPort() == 80 && srv3.getServerNames().size() == 1 &&
       srv3.getLocations().size() == 1 &&
       srv3.getLocations()[0].getPath() == "/api");

  // Verify deep copy of locations
  loc.setPath("/changed");
  srv1.setLocations(std::vector<LocationConfig>());
  srv1.addLocation(loc);

  bool deep_pass = (srv2.getLocations()[0].getPath() == "/api");

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
    loc.setPath(ss.str());
    srv.addLocation(loc);
  }

  bool massive_pass = (srv.getLocations().size() == 500 &&
                       srv.getLocations().back().getPath() == "/path-499");

  print_result("test_edge_cases", massive_pass);
}

void test_stress() {
  std::cout << "[Test] Stress test: 30,000 deep instantiations..." << std::endl;
  for (int i = 0; i < 30000; ++i) {
    ServerConfig srv;
    srv.setPort(i % 65535);

    for (int j = 0; j < 5; ++j) {
      LocationConfig loc;
      loc.setPath("/loc");
      srv.addLocation(loc);
    }

    ServerConfig copy(srv);
    ServerConfig assign;
    assign = copy;

    if (assign.getLocations().size() != 5) {
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
  loc_root.setPath("/");
  srv.addLocation(loc_root);

  LocationConfig loc_api;
  loc_api.setPath("/api");
  srv.addLocation(loc_api);

  LocationConfig loc_users;
  loc_users.setPath("/api/users");
  srv.addLocation(loc_users);

  LocationConfig loc_images;
  loc_images.setPath("/images/");
  srv.addLocation(loc_images);

  bool pass = true;

  // 1. Exact Match
  const LocationConfig* res = srv.findLocation("/api");
  if (!res || res->getPath() != "/api") pass = false;

  // 2. Prefix Match
  res = srv.findLocation("/api/docs");
  if (!res || res->getPath() != "/api") pass = false;

  // 3. Longest Prefix Wins
  res = srv.findLocation("/api/users/profile");
  if (!res || res->getPath() != "/api/users") pass = false;

  // 4. Strict Boundary Check (Failure Case)
  // "/api_v2" starts with "/api" but is not a subpath.
  // Should match root "/" or NULL.
  res = srv.findLocation("/api_v2");
  if (!res || res->getPath() != "/") pass = false;

  // 5. Trailing Slash Handling
  res = srv.findLocation("/images/logo.png");
  if (!res || res->getPath() != "/images/") pass = false;

  // 6. Root Fallback
  res = srv.findLocation("/something_else");
  if (!res || res->getPath() != "/") pass = false;

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
