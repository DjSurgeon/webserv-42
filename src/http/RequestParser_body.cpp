// Copyright 2026 serjimen vja-nie dlesieur
#include <map>
#include <sstream>
#include <string>

#include "http/RequestParser.hpp"

/**
 * @brief Decides transition to STATE_BODY or
 * STATE_COMPLETE based on Content-Length.
 */
void RequestParser::_determine_body_transition() {
  const std::map<std::string, std::string>& headers = _request.get_headers();
  std::map<std::string, std::string>::const_iterator it =
      headers.find("content-length");
  if (it == headers.end()) {
    _state = STATE_COMPLETE;
    return;
  }

  std::string val = it->second;
  if (val.empty()) {
    _state = STATE_COMPLETE;
    return;
  }

  for (size_t i = 0; i < val.size(); ++i) {
    if (val[i] < '0' || val[i] > '9') {
      _state = STATE_ERROR;
      return;
    }
  }

  std::stringstream ss(val);
  size_t len = 0;
  if (!(ss >> len)) {
    _state = STATE_ERROR;
    return;
  }

  if (len == 0) {
    _state = STATE_COMPLETE;
  } else {
    _content_length = len;
    _storage_buffer.clear();
    _state = STATE_BODY;
  }
}

/**
 * @brief Accumulates characters of the HTTP request body.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_state_body(char c) {
  _storage_buffer += c;
  if (_storage_buffer.size() == _content_length) {
    _request.set_body(_storage_buffer);
    _storage_buffer.clear();
    _state = STATE_COMPLETE;
  }
}
