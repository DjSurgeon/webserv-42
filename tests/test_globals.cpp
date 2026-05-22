// Copyright 2026 serjimen vja-nie dlesieur
#include <csignal>

/**
 * @brief Global flag to control the EventLoop execution in test environments.
 *
 * Defined here to satisfy linker requirements for unit tests that compile
 * EventLoop.cpp but do not include main.cpp.
 */
volatile sig_atomic_t g_running = 1;
