// Copyright 2026 serjimen vja-nie dlesieur
#include <sys/stat.h>
#include <unistd.h>

#include <cassert>
#include <fstream>
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

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static void create_test_file(const std::string& path,
                             const std::string& content) {
  std::ofstream file(path.c_str());
  file << content;
  file.close();
}

static void delete_test_file(const std::string& path) {
  unlink(path.c_str());
}

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

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

void test_serve_file_success() {
  std::cout << "[Test] Verifying successful file serving (200 OK)..."
            << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/test_ok.txt";
  std::string content = "Hello Webserv!";
  create_test_file(path, content);

  // 2. ACT
  bool result = handler.serve_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  assert(res_str.find("HTTP/1.1 200 OK") != std::string::npos);
  assert(res_str.find("Content-Type: text/plain") != std::string::npos);
  assert(res_str.find("Content-Length: 14") != std::string::npos);
  assert(res_str.find(content) != std::string::npos);

  delete_test_file(path);
  print_result("test_serve_file_success", true);
}

void test_serve_file_not_found() {
  std::cout << "[Test] Verifying file not found (404)..." << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/non_existent_404_test.txt";
  delete_test_file(path);  // Ensure it doesn't exist

  // 2. ACT
  bool result = handler.serve_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  assert(res_str.find("404 Not Found") != std::string::npos);

  print_result("test_serve_file_not_found", true);
}

void test_serve_file_forbidden() {
  std::cout << "[Test] Verifying permission denied (403)..." << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/test_forbidden.txt";
  create_test_file(path, "Top Secret");
  chmod(path.c_str(), 0000);  // Remove all permissions

  // 2. ACT
  bool result = handler.serve_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  assert(res_str.find("403 Forbidden") != std::string::npos);

  // Cleanup: Restore permissions so we can delete it
  chmod(path.c_str(), 0644);
  delete_test_file(path);
  print_result("test_serve_file_forbidden", true);
}

void test_serve_directory_forbidden() {
  std::cout << "[Test] Verifying directory access is Forbidden (403)..."
            << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/";  // Path to a directory

  // 2. ACT
  bool result = handler.serve_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  // NGINX behavior: Accessing a directory returns 403 Forbidden
  assert(res_str.find("403 Forbidden") != std::string::npos);

  print_result("test_serve_directory_forbidden", true);
}

void test_delete_file_not_found() {
  std::cout << "[Test] Verifying delete non-existent file returns 404..."
            << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/var/www/fake_file.txt";

  // 2. ACT
  bool result = handler.delete_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  assert(res_str.find("404 Not Found") != std::string::npos);

  print_result("test_delete_file_not_found", true);
}

void test_delete_directory_forbidden() {
  std::cout << "[Test] Verifying delete directory returns 403..." << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/test_del_dir";
  mkdir(path.c_str(), 0755);

  // 2. ACT
  bool result = handler.delete_file(path, res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(result == true);
  assert(res_str.find("403 Forbidden") != std::string::npos);
  // Verify directory STILL exists
  assert(access(path.c_str(), F_OK) == 0);

  // Cleanup
  rmdir(path.c_str());
  print_result("test_delete_directory_forbidden", true);
}

void test_autoindex_success() {
  std::cout << "[Test] Verifying successful autoindex generation (200 OK)..."
            << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string dir_path = "/tmp/test_autoindex/";
  mkdir(dir_path.c_str(), 0755);
  create_test_file(dir_path + "file1.txt", "content1");
  create_test_file(dir_path + "file2.html", "content2");

  // 2. ACT
  handler.generate_autoindex(dir_path, "/auto/", res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  assert(res_str.find("HTTP/1.1 200 OK") != std::string::npos);
  assert(res_str.find("Content-Type: text/html") != std::string::npos);
  assert(res_str.find("<title>Index of /auto/</title>") != std::string::npos ||
         res_str.find("<title>Index of /auto</title>") != std::string::npos);
  assert(res_str.find("file1.txt") != std::string::npos);
  assert(res_str.find("file2.html") != std::string::npos);
  assert(res_str.find("href=\"/auto/file1.txt\"") != std::string::npos);

  // Cleanup
  delete_test_file(dir_path + "file1.txt");
  delete_test_file(dir_path + "file2.html");
  rmdir(dir_path.c_str());

  print_result("test_autoindex_success", true);
}

void test_autoindex_forbidden() {
  std::cout << "[Test] Verifying autoindex failure on non-existent dir (403)..."
            << std::endl;
  // 1. ARRANGE
  FileHandler handler;
  HttpResponse res;
  std::string path = "/tmp/non_existent_autoindex_dir/";

  // 2. ACT
  handler.generate_autoindex(path, "/auto/", res);
  std::string res_str = res.to_string();

  // 3. ASSERT
  // Logic in FileHandler returns 403 if opendir fails
  assert(res_str.find("403 Forbidden") != std::string::npos);

  print_result("test_autoindex_forbidden", true);
}

int main() {
  std::cout << "=== STARTING FILE HANDLER COMPREHENSIVE TESTS ===\n"
            << std::endl;

  test_get_mime_type();
  std::cout << std::endl;
  test_serve_file_success();
  test_serve_file_not_found();
  test_serve_file_forbidden();
  test_serve_directory_forbidden();
  test_delete_file_not_found();
  test_delete_directory_forbidden();
  std::cout << std::endl;
  test_autoindex_success();
  test_autoindex_forbidden();

  std::cout << "\n=== FILE HANDLER TESTS COMPLETED ===" << std::endl;
  return 0;
}
