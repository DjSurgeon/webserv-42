// Copyright 2026 serjimen vja-nie dlesieur
#include "http/SessionManager.hpp"

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

SessionManager& SessionManager::get_instance() {
  static SessionManager instance;
  return instance;
}

SessionManager::SessionManager() {
  // Initialize random seed exactly once
  std::srand(static_cast<unsigned int>(std::time(NULL)));
}

SessionManager::~SessionManager() {
  _active_sessions.clear();
}

std::string SessionManager::_generate_random_id() const {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";

  std::string id;
  id.reserve(32);
  for (int i = 0; i < 32; ++i) {
    id += alphanum[std::rand() % (sizeof(alphanum) - 1)];
  }
  return id;
}

std::string SessionManager::create_session(const std::string& username,
                                           time_t ttl) {
  // Purge old sessions first to prevent memory bloat over time
  clear_expired_sessions();

  std::string session_id = _generate_random_id();

  // Make sure ID is absolutely unique (extremely rare collision check)
  while (_active_sessions.find(session_id) != _active_sessions.end()) {
    session_id = _generate_random_id();
  }

  SessionData data;
  data.username = username;
  data.expires_at = std::time(NULL) + ttl;

  _active_sessions[session_id] = data;

  return session_id;
}
SessionData* SessionManager::get_session(const std::string& session_id) {
  std::map<std::string, SessionData>::iterator it =
      _active_sessions.find(session_id);

  if (it == _active_sessions.end()) {
    return NULL;
  }

  // Check if session has expired
  if (std::time(NULL) > it->second.expires_at) {
    _active_sessions.erase(it);
    return NULL;
  }

  return &(it->second);
}

void SessionManager::destroy_session(const std::string& session_id) {
  std::map<std::string, SessionData>::iterator it =
      _active_sessions.find(session_id);

  if (it != _active_sessions.end()) {
    _active_sessions.erase(it);
  }
}

void SessionManager::clear_expired_sessions() {
  time_t now = std::time(NULL);
  std::vector<std::string> to_remove;

  std::map<std::string, SessionData>::iterator it;
  for (it = _active_sessions.begin(); it != _active_sessions.end(); ++it) {
    if (now > it->second.expires_at) {
      to_remove.push_back(it->first);
    }
  }

  for (size_t i = 0; i < to_remove.size(); ++i) {
    _active_sessions.erase(to_remove[i]);
  }
}
