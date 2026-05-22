// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "config/LocationConfig.hpp"
#include "handlers/StaticRouter.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"

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

void test_null_location() {
  std::cout << "[Test] Verifying NULL location handling..." << std::endl;
  StaticRouter router;
  HttpRequest req;
  HttpResponse res;
  std::string path;

  bool result = router.process_route(req, NULL, &res, &path);

  // Should return false and set 404
  bool pass = (result == false &&
               res.to_string().find("404 Not Found") != std::string::npos);
  print_result("test_null_location", pass);
}

void test_method_validation() {
  std::cout << "[Test] Verifying method validation..." << std::endl;
  StaticRouter router;
  LocationConfig loc;
  loc.set_root("/var/www");

  // Test 1: Default GET allowed
  {
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    assert(router.process_route(req, &loc, &res, &path) == true);

    req.set_method("POST");
    assert(router.process_route(req, &loc, &res, &path) == false);
    assert(res.to_string().find("405 Method Not Allowed") != std::string::npos);
  }

  // Test 2: Custom allowed methods
  {
    loc.add_allowed_method("POST");
    loc.add_allowed_method("DELETE");

    HttpRequest req;
    req.set_method("POST");
    req.set_uri("/upload");
    HttpResponse res;
    std::string path;
    assert(router.process_route(req, &loc, &res, &path) == true);

    req.set_method("GET");
    assert(router.process_route(req, &loc, &res, &path) == false);
  }

  print_result("test_method_validation", true);
}

void test_path_translation() {
  std::cout << "[Test] Verifying path translation logic..." << std::endl;
  StaticRouter router;

  // Test 1: Standard join
  {
    LocationConfig loc;
    loc.set_root("/var/www");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    router.process_route(req, &loc, &res, &path);
    assert(path == "/var/www/index.html");
  }

  // Test 2: Trailing slash in root
  {
    LocationConfig loc;
    loc.set_root("/var/www/");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    router.process_route(req, &loc, &res, &path);
    assert(path == "/var/www/index.html");
  }

  // Test 3: No root
  {
    LocationConfig loc;
    loc.set_root("");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/css/style.css");
    HttpResponse res;
    std::string path;
    router.process_route(req, &loc, &res, &path);
    assert(path == "/css/style.css");
  }

  print_result("test_path_translation", true);
}

void test_stress() {
  std::cout << "[Test] Stress test: 100,000 routing operations..." << std::endl;
  StaticRouter router;
  LocationConfig loc;
  loc.set_root("/very/long/path/to/some/deep/directory/structure");
  loc.add_allowed_method("GET");

  HttpRequest req;
  req.set_method("GET");
  req.set_uri("/some/complex/uri/with/many/segments/and/data.txt");

  for (int i = 0; i < 100000; ++i) {
    HttpResponse res;
    std::string path;
    if (!router.process_route(req, &loc, &res, &path)) {
      print_result("test_stress", false);
      return;
    }
  }
  print_result("test_stress", true);
}

int main() {
  std::cout << "=== STARTING STATIC ROUTER TESTS ===\n" << std::endl;

  test_null_location();
  std::cout << std::endl;
  test_method_validation();
  std::cout << std::endl;
  test_path_translation();
  std::cout << std::endl;
  test_stress();

  std::cout << "\n=== STATIC ROUTER TESTS COMPLETED ===" << std::endl;
  return 0;
}
