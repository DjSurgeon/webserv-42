// Copyright 2026 raperez- serjimen
#include "http/RequestParser.hpp"

/**
 * @brief Helper utility to check if a character is alphabetical.
 *
 * @param c The character to check.
 * @return True if the character is in [A-Za-z], false otherwise.
 */
static bool is_alpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/**
 * @brief Helper utility to check if a character
 * is a valid non-space URI character.
 *
 * @param c The character to check.
 * @return True if character is printable visible
 * ASCII and non-space, false otherwise.
 */
static bool is_uri_char(char c) {
  return c > 32 && c < 127;
}

/**
 * @brief Evaluates start state characters,
 * discarding leading spaces and line feeds.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_start(char c) {
  if (c == ' ' || c == '\r' || c == '\n') {
    return;
  }
  if (is_alpha(c)) {
    _storage_buffer.clear();
    _storage_buffer += c;
    _state = STATE_METHOD;
  } else {
    _state = STATE_ERROR;
  }
}

/**
 * @brief Parses the HTTP method name, accumulating alphabetical tokens.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_method(char c) {
  if (is_alpha(c)) {
    if (_storage_buffer.size() >= 16) {
      _state = STATE_ERROR;
      return;
    }
    _storage_buffer += c;
  } else if (c == ' ') {
    _request.set_method(_storage_buffer);
    _storage_buffer.clear();
    _state = STATE_URI;
  } else {
    _state = STATE_ERROR;
  }
}

/**
 * @brief Parses the target URI path, validating characters and delimiters.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_uri(char c) {
  if (c == ' ') {
    if (_storage_buffer.empty()) {
      _state = STATE_ERROR;
    } else {
      size_t double_slash;
      while ((double_slash = _storage_buffer.find("//")) != std::string::npos) {
        _storage_buffer.erase(double_slash, 1);
      }
      _request.set_uri(_storage_buffer);
      _storage_buffer.clear();
      _state = STATE_VERSION;
    }
  } else if (is_uri_char(c)) {
    if (_storage_buffer.size() >= 8192) {
      _state = STATE_ERROR;
      return;
    }
    _storage_buffer += c;
  } else {
    _state = STATE_ERROR;
  }
}

/**
 * @brief Strictly validates the HTTP protocol version string "HTTP/1.1".
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_version(char c) {
  if (c == '\r') {
    if (_storage_buffer == "HTTP/1.1") {
      _request.set_version(_storage_buffer);
      _storage_buffer.clear();
      _expect_newline = true;
    } else {
      _state = STATE_ERROR;
    }
  } else {
    size_t len = _storage_buffer.size();
    if (len < 8 && c == "HTTP/1.1"[len]) {
      _storage_buffer += c;
    } else {
      _state = STATE_ERROR;
    }
  }
}
