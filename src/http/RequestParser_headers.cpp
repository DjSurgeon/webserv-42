// Copyright 2026 serjimen vja-nie dlesieur
#include "http/RequestParser.hpp"

#include <string>

/**
 * @brief Helper utility to check if a character
 * is a valid header key token character.
 *
 * @param c The character to check.
 * @return True if alphanumeric or hyphen.
 */
static bool is_header_key_char(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
         (c >= '0' && c <= '9') || c == '-';
}

/**
 * @brief Trims spaces and tabs from both ends
 * of a string.
 *
 * @param str The string to trim.
 * @return The trimmed string.
 */
static std::string trim_spaces(const std::string& str) {
  size_t start = 0;
  while (start < str.size() && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  if (start == str.size()) {
    return "";
  }
  size_t end = str.size() - 1;
  while (end > start && (str[end] == ' ' || str[end] == '\t')) {
    end--;
  }
  return str.substr(start, end - start + 1);
}

/**
 * @brief Parses individual characters of a header
 * name, transitioning to value on colon.
 *
 * Also detects double CRLF sequences marking the end of the headers block.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_header_key(char c) {
  if (c == ':') {
    if (_storage_buffer.empty()) {
      _state = STATE_ERROR;
    } else {
      _current_header_key = _storage_buffer;
      _storage_buffer.clear();
      _state = STATE_HEADER_VALUE;
    }
  } else if (c == '\r') {
    if (_storage_buffer.empty()) {
      _expect_newline = true;
    } else {
      _state = STATE_ERROR;
    }
  } else if (is_header_key_char(c)) {
    if (_storage_buffer.size() >= 1024) {
      _state = STATE_ERROR;
      return;
    }
    _storage_buffer += c;
  } else {
    _state = STATE_ERROR;
  }
}

/**
 * @brief Parses individual characters of a header
 * value, trimming surrounding space and tabs.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_header_value(char c) {
  if (c == '\r') {
    if (_request.get_headers().size() >= 100) {
      _state = STATE_ERROR;
      return;
    }
    std::string trimmed = trim_spaces(_storage_buffer);
    _request.add_header(_current_header_key, trimmed);
    _storage_buffer.clear();
    _current_header_key.clear();
    _expect_newline = true;
  } else if (c == ' ' || c == '\t' || (c >= 33 && c <= 126) ||
             (unsigned char)c >= 128) {
    if (_storage_buffer.size() >= 8192) {
      _state = STATE_ERROR;
      return;
    }
    _storage_buffer += c;
  } else {
    _state = STATE_ERROR;
  }
}
