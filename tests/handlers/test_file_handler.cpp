// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <string>

#include "handlers/FileHandler.hpp"

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

void test_get_mime_type() {
  // 1. ARRANGE
  FileHandler handler;

  // 2. ACT & 3. ASSERT (Standard types)
  std::cout << "[Test] Verifying standard MIME types..." << std::endl;
  assert(handler._get_mime_type("index.html") == "text/html");
  assert(handler._get_mime_type("style.css") == "text/css");
  assert(handler._get_mime_type("script.js") == "application/javascript");
  assert(handler._get_mime_type("image.png") == "image/png");
  assert(handler._get_mime_type("photo.jpg") == "image/jpeg");
  assert(handler._get_mime_type("photo.jpeg") == "image/jpeg");
  assert(handler._get_mime_type("note.txt") == "text/plain");
  print_result("test_standard_mime_types", true);

  // 2. ACT & 3. ASSERT (Edge cases: No extension)
  std::cout << "[Test] Verifying files without extension..." << std::endl;
  assert(handler._get_mime_type("Makefile") == "application/octet-stream");
  assert(handler._get_mime_type("plain_file") == "application/octet-stream");
  assert(handler._get_mime_type("folder.dot/") == "application/octet-stream");
  print_result("test_no_extension", true);

  // 2. ACT & 3. ASSERT (Edge cases: Multiple dots)
  std::cout << "[Test] Verifying multiple dots..." << std::endl;
  assert(handler._get_mime_type("archive.tar.gz") ==
         "application/octet-stream");
  assert(handler._get_mime_type("backup.2026.html") == "text/html");
  print_result("test_multiple_dots", true);

  // 2. ACT & 3. ASSERT (Edge cases: Unknown extension)
  std::cout << "[Test] Verifying unknown extensions..." << std::endl;
  assert(handler._get_mime_type("data.json") == "application/octet-stream");
  assert(handler._get_mime_type("image.webp") == "application/octet-stream");
  print_result("test_unknown_extension", true);
}

int main() {
  std::cout << "=== STARTING FILE HANDLER MIME TESTS ===\n" << std::endl;

  test_get_mime_type();

  std::cout << "\n=== FILE HANDLER TESTS COMPLETED ===" << std::endl;
  return 0;
}
