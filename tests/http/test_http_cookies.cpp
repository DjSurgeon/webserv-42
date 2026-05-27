// Copyright 2026 serjimen vja-nie dlesieur
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

#include "http/HttpRequest.hpp"

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
  std::cout << "[Test] Verifying multiple cookies separated by ;..."
            << std::endl;
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
  std::cout << "[Test] Verifying resistance against extra spaces..."
            << std::endl;
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

void test_corrupted_cookies() {
  std::cout << "[Test] Verifying resistance against corrupted headers..." << std::endl;
  HttpRequest req;
  req.add_header("Cookie", "session=123; ; ;; path=/");

  const std::map<std::string, std::string>& cookies = req.get_cookies();
  bool pass = true;

  if (cookies.size() != 2) pass = false;
  if (cookies.find("session") == cookies.end() || cookies.find("session")->second != "123") pass = false;
  if (cookies.find("path") == cookies.end() || cookies.find("path")->second != "/") pass = false;

  print_result("test_corrupted_cookies", pass);
}

void test_empty_names() {
  std::cout << "[Test] Verifying resistance against empty assignments..." << std::endl;
  HttpRequest req;
  req.add_header("Cookie", "=value; name=");

  const std::map<std::string, std::string>& cookies = req.get_cookies();
  bool pass = true;

  // The parser should safely handle these without crashing.
  // We expect empty key for "=value", and "name" for empty value.
  if (cookies.find("") == cookies.end() || cookies.find("")->second != "value") pass = false;
  if (cookies.find("name") == cookies.end() || cookies.find("name")->second != "") pass = false;

  print_result("test_empty_names", pass);
}

void test_special_chars() {
  std::cout << "[Test] Verifying special characters..." << std::endl;
  HttpRequest req;
  req.add_header("Cookie", "user@name=serjimen!");

  const std::map<std::string, std::string>& cookies = req.get_cookies();
  bool pass = true;

  if (cookies.size() != 1) pass = false;
  if (cookies.find("user@name") == cookies.end() || cookies.find("user@name")->second != "serjimen!") pass = false;

  print_result("test_special_chars", pass);
}

void test_massive_cookies() {
  std::cout << "[Test] Verifying Buffer Overflow resistance (Massive Cookie Bombing)..." << std::endl;
  HttpRequest req;
  
  std::string massive_payload = "";
  for (int i = 0; i < 1000; i++) {
    std::stringstream ss_k, ss_v;
    ss_k << "key" << i;
    ss_v << "value" << i;
    massive_payload += ss_k.str() + "=" + ss_v.str() + "; ";
  }
  
  req.add_header("Cookie", massive_payload);
  const std::map<std::string, std::string>& cookies = req.get_cookies();
  
  bool pass = true;
  if (cookies.size() != 1000) pass = false;
  
  std::map<std::string, std::string>::const_iterator it = cookies.find("key500");
  if (it == cookies.end() || it->second != "value500") pass = false;

  print_result("test_massive_cookies", pass);
}

int main() {
  std::cout << "=== STARTING HTTP COOKIES TESTS ===\n" << std::endl;

  test_single_cookie();
  std::cout << std::endl;
  test_multiple_cookies();
  std::cout << std::endl;
  test_cookies_with_extra_spaces();
  std::cout << std::endl;
  test_corrupted_cookies();
  std::cout << std::endl;
  test_empty_names();
  std::cout << std::endl;
  test_special_chars();
  std::cout << std::endl;
  test_massive_cookies();

  std::cout << "\n=== HTTP COOKIES TESTS COMPLETED ===" << std::endl;
  return 0;
}
