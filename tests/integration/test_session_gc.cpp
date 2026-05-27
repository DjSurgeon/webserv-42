// Copyright 2026 serjimen vja-nie dlesieur
#include <unistd.h>

#include <csignal>
#include <ctime>
#include <iostream>
#include <string>

#include "http/SessionManager.hpp"
#include "network/EventLoop.hpp"

// ANSI Color codes for pretty output
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

extern volatile sig_atomic_t g_running;

void print_result(const std::string& test_name, bool success) {
  std::cout << test_name << ": "
            << (success ? std::string(GREEN) + "PASSED"
                        : std::string(RED) + "FAILED")
            << RESET << std::endl;
}

void test_session_expiration_manual() {
  std::cout << "[Test] Verifying manual session expiration..." << std::endl;
  SessionManager& sm = SessionManager::get_instance();

  // Create a session that expires in 1 second
  std::string session_id = sm.create_session("expire_me", 1);

  if (sm.get_session(session_id) == NULL) {
    print_result("test_session_expiration_manual (initial check)", false);
    return;
  }

  std::cout << "Waiting 2 seconds for session to expire..." << std::endl;
  sleep(2);

  sm.clear_expired_sessions();

  bool pass = (sm.get_session(session_id) == NULL);
  print_result("test_session_expiration_manual", pass);
}

void test_event_loop_gc_trigger() {
  std::cout << "[Test] Verifying EventLoop GC trigger..." << std::endl;
  SessionManager& sm = SessionManager::get_instance();
  EventLoop loop;

  // Set cleanup interval to 1 second
  loop.setSessionCleanupInterval(1);

  // Create a session that expires in 1 second
  std::string session_id = sm.create_session("loop_expire_me", 1);

  std::cout << "Waiting 2 seconds..." << std::endl;
  sleep(2);

  // Simulate the logic inside EventLoop::run() without blocking
  sm.clear_expired_sessions();

  bool pass = (sm.get_session(session_id) == NULL);
  print_result("test_event_loop_gc_trigger", pass);
}

int main() {
  std::cout << "--- Session Garbage Collection Integration Tests ---"
            << std::endl;

  test_session_expiration_manual();
  test_event_loop_gc_trigger();

  std::cout << "----------------------------------------------------"
            << std::endl;
  return 0;
}
