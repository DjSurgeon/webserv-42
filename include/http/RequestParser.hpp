// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_HTTP_REQUESTPARSER_HPP_
#define INCLUDE_HTTP_REQUESTPARSER_HPP_

#include <cstddef>
#include <string>

#include "http/HttpRequest.hpp"

/**
 * @brief State-machine enum for HTTP request parsing.
 */
enum e_parser_state {
  STATE_START,
  STATE_METHOD,
  STATE_URI,
  STATE_VERSION,
  STATE_HEADER_KEY,
  STATE_HEADER_VALUE,
  STATE_BODY,
  STATE_COMPLETE,
  STATE_ERROR,
  STATE_CHUNK_SIZE,
  STATE_CHUNK_DATA,
  STATE_CHUNK_CRLF,
  STATE_CHUNK_END
};

/**
 * @brief FSM HTTP request stream parser.
 *
 * Processes a request character by character using a Finite State Machine.
 */
class RequestParser {
 public:
  RequestParser();
  ~RequestParser();

  e_parser_state feed(char c);
  e_parser_state get_state() const;
  const HttpRequest& get_request() const;
  void reset();

 private:
  e_parser_state _state;
  HttpRequest _request;
  bool _expect_newline;
  std::string _storage_buffer;
  std::string _current_header_key;
  size_t _content_length;
  size_t _chunk_size;
  size_t _chunk_read;
  std::string _chunk_size_buffer;

  // --- Private State Handlers (The Refactored Switch Delegation) ---
  void _handle_state_start(char c);
  void _handle_state_method(char c);
  void _handle_state_uri(char c);
  void _handle_state_version(char c);
  void _handle_state_header_key(char c);
  void _handle_state_header_value(char c);
  void _handle_state_body(char c);
  void _handle_state_chunk_size(char c);
  void _handle_state_chunk_data(char c);
  void _handle_state_chunk_crlf(char c);
  void _handle_state_chunk_end(char c);
  void _determine_body_transition();
  void _handle_global_newline(char c);

  // Prevent copying (C++98 style)
  RequestParser(const RequestParser& other);
  RequestParser& operator=(const RequestParser& other);
};

#endif  // INCLUDE_HTTP_REQUESTPARSER_HPP_
