// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HTTP_SESSIONMANAGER_HPP_
#define SRC_HTTP_SESSIONMANAGER_HPP_

#include <ctime>
#include <map>
#include <string>

struct SessionData {
  std::string username;
  time_t expires_at;
};

class SessionManager {
 public:
  // Singleton Pattern: global access point
  static SessionManager& get_instance();

  std::string create_session(const std::string& username);
  SessionData* get_session(const std::string& session_id);
  void destroy_session(const std::string& session_id);
  void clear_expired_sessions();

 private:
  // Private constructor and destructor to prevent instantiation
  SessionManager();
  ~SessionManager();

  // Prevent copying and assignment
  SessionManager(const SessionManager& other);
  SessionManager& operator=(const SessionManager& other);

  std::string _generate_random_id() const;

  std::map<std::string, SessionData> _active_sessions;
};

#endif  // SRC_HTTP_SESSIONMANAGER_HPP_
