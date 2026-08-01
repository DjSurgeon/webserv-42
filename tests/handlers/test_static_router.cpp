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

  bool result = router.process_route(req, NULL, NULL, &res, &path, NULL);

  // Should return false and set 404
  bool pass = (result == false &&
               res.to_string().find("404 Not Found") != std::string::npos);
  print_result("test_null_location", pass);
}

void test_method_validation() {
  std::cout << "[Test] Verifying method validation..." << std::endl;
  StaticRouter router;
  LocationConfig loc;
  loc.setRoot("/var/www");

  // Test 1: Default GET allowed
  {
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    assert(router.process_route(req, NULL, &loc, &res, &path, NULL) == true);

    req.set_method("POST");
    assert(router.process_route(req, NULL, &loc, &res, &path, NULL) == false);
    assert(res.to_string().find("405 Method Not Allowed") != std::string::npos);
  }

  // Test 2: Custom allowed methods
  {
    loc.addAllowedMethod("POST");
    loc.addAllowedMethod("DELETE");

    HttpRequest req;
    req.set_method("POST");
    req.set_uri("/upload");
    HttpResponse res;
    std::string path;
    assert(router.process_route(req, NULL, &loc, &res, &path, NULL) == true);

    req.set_method("GET");
    assert(router.process_route(req, NULL, &loc, &res, &path, NULL) == false);
  }

  print_result("test_method_validation", true);
}

void test_path_translation() {
  std::cout << "[Test] Verifying path translation logic..." << std::endl;
  StaticRouter router;

  // Test 1: Standard join
  {
    LocationConfig loc;
    loc.setRoot("/var/www");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    router.process_route(req, NULL, &loc, &res, &path, NULL);
    assert(path == "/var/www/index.html");
  }

  // Test 2: Trailing slash in root
  {
    LocationConfig loc;
    loc.setRoot("/var/www/");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    HttpResponse res;
    std::string path;
    router.process_route(req, NULL, &loc, &res, &path, NULL);
    assert(path == "/var/www/index.html");
  }

  // Test 3: No root
  {
    LocationConfig loc;
    loc.setRoot("");
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/css/style.css");
    HttpResponse res;
    std::string path;
    router.process_route(req, NULL, &loc, &res, &path, NULL);
    assert(path == "/css/style.css");
  }

  print_result("test_path_translation", true);
}

void test_stress() {
  std::cout << "[Test] Stress test: 100,000 routing operations..." << std::endl;
  StaticRouter router;
  LocationConfig loc;
  loc.setRoot("/very/long/path/to/some/deep/directory/structure");
  loc.addAllowedMethod("GET");

  HttpRequest req;
  req.set_method("GET");
  req.set_uri("/some/complex/uri/with/many/segments/and/data.txt");

  for (int i = 0; i < 100000; ++i) {
    HttpResponse res;
    std::string path;
    if (!router.process_route(req, NULL, &loc, &res, &path, NULL)) {
      print_result("test_stress", false);
      return;
    }
  }
  print_result("test_stress", true);
}

void test_path_traversal_attack() {
  std::cout << "[Test] Verifying Path Traversal (LFI) attack mitigation..."
            << std::endl;
  // 1. ARRANGE
  StaticRouter router;
  LocationConfig loc;
  loc.setRoot("/var/www");
  HttpRequest req;
  req.set_method("GET");
  req.set_uri("/../../../../etc/passwd");
  HttpResponse res;
  std::string path;

  // 2. ACT
  // Note: We expect the router to EITHER sanitize the path OR return an error
  // status.
  router.process_route(req, NULL, &loc, &res, &path, NULL);

  // 3. ASSERT
  // Failure condition: If path contains "/etc/passwd" and doesn't start with
  // root, it's a vulnerability.
  bool contains_passwd = (path.find("/etc/passwd") != std::string::npos);
  bool outside_root = (path.find("/var/www") != 0);

  if (contains_passwd && outside_root) {
    std::cout << RED << "VULNERABILITY DETECTED: Router allowed path: " << path
              << RESET << std::endl;
    print_result("test_path_traversal_attack", false);
  } else {
    print_result("test_path_traversal_attack", true);
  }
}

void test_i18n_content_negotiation() {
  std::cout << "[Test] Verifying i18n content negotiation..." << std::endl;
  StaticRouter router;
  LocationConfig loc;
  loc.setRoot("www/eval/html");

  // Case 1: Spanish preference
  {
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    req.add_header("Accept-Language", "es-ES, es;q=0.9, en;q=0.8");
    HttpResponse res;
    std::string path;
    router.process_route(req, NULL, &loc, &res, &path, NULL);
    bool pass = (path == "www/eval/html/index.html.es");
    if (!pass) std::cerr << "Expected index.html.es, got " << path << std::endl;
    print_result("  -> Spanish preference", pass);
  }

  // Case 2: English preference
  {
    HttpRequest req;
    req.set_method("GET");
    req.set_uri("/index.html");
    req.add_header("Accept-Language", "en, es;q=0.5");
    HttpResponse res;
    std::string path;
    router.process_route(req, NULL, &loc, &res, &path, NULL);
    bool pass = (path == "www/eval/html/index.html.en");
    if (!pass) std::cerr << "Expected index.html.en, got " << path << std::endl;
    print_result("  -> English preference", pass);
  }

  print_result("test_i18n_content_negotiation", true);
}

int main() {
  std::cout << "=== STARTING STATIC ROUTER TESTS ===\n" << std::endl;

  test_null_location();
  std::cout << std::endl;
  test_method_validation();
  std::cout << std::endl;
  test_path_translation();
  std::cout << std::endl;
  test_path_traversal_attack();
  std::cout << std::endl;
  test_i18n_content_negotiation();
  std::cout << std::endl;
  test_stress();

  std::cout << "\n=== STATIC ROUTER TESTS COMPLETED ===" << std::endl;
  return 0;
}
