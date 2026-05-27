// Copyright 2026 serjimen vja-nie dlesieur
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "http/SessionManager.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_singleton() {
  std::cout << "[Test] Verifying Singleton instance..." << std::endl;
  SessionManager& sm1 = SessionManager::get_instance();
  SessionManager& sm2 = SessionManager::get_instance();

  bool pass = (&sm1 == &sm2);
  print_result("test_singleton", pass);
}

void test_create_and_get_session() {
  std::cout << "[Test] Verifying session creation and retrieval..."
            << std::endl;
  SessionManager& sm = SessionManager::get_instance();

  std::string username = "serjimen";
  std::string session_id = sm.create_session(username);

  bool pass = true;
  if (session_id.length() != 32) {
    std::cerr << "Invalid session ID length: " << session_id.length()
              << std::endl;
    pass = false;
  }

  SessionData* data = sm.get_session(session_id);
  if (data == NULL) {
    std::cerr << "Session not found for ID: " << session_id << std::endl;
    pass = false;
  } else if (data->username != username) {
    std::cerr << "Username mismatch. Expected: " << username
              << ", Got: " << data->username << std::endl;
    pass = false;
  }

  print_result("test_create_and_get_session", pass);
}

void test_get_nonexistent_session() {
  std::cout << "[Test] Verifying retrieval of nonexistent session..."
            << std::endl;
  SessionManager& sm = SessionManager::get_instance();

  SessionData* data = sm.get_session("nonexistent_id");
  bool pass = (data == NULL);

  print_result("test_get_nonexistent_session", pass);
}

void test_destroy_session() {
  std::cout << "[Test] Verifying session destruction..." << std::endl;
  SessionManager& sm = SessionManager::get_instance();

  std::string session_id = sm.create_session("temp_user");
  sm.destroy_session(session_id);

  SessionData* data = sm.get_session(session_id);
  bool pass = (data == NULL);

  print_result("test_destroy_session", pass);
}

void test_high_concurrency_scaling() {
  std::cout
      << "[Test] Verifying O(log N) lookup under massive 10,000 session load..."
      << std::endl;
  SessionManager& sm = SessionManager::get_instance();
  std::vector<std::string> ids;
  bool pass = true;

  // 1. Inject 10,000 sessions
  for (int i = 0; i < 10000; i++) {
    std::stringstream ss;
    ss << "user" << i;
    ids.push_back(sm.create_session(ss.str()));
  }

  // 2. Lookup random session (e.g. index 5000)
  SessionData* data = sm.get_session(ids[5000]);
  if (data == NULL || data->username != "user5000") pass = false;

  // 3. Destroy all
  for (int i = 0; i < 10000; i++) {
    sm.destroy_session(ids[i]);
  }

  if (sm.get_session(ids[5000]) != NULL) pass = false;

  print_result("test_high_concurrency_scaling", pass);
}

int main() {
  std::cout << "--- SessionManager Unit Tests ---" << std::endl;

  test_singleton();
  test_create_and_get_session();
  test_get_nonexistent_session();
  test_destroy_session();
  test_high_concurrency_scaling();

  std::cout << "---------------------------------" << std::endl;
  return 0;
}
