// Copyright 2026 raperez- serjimen
#include <cctype>
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
  std::map<std::string, std::string>::const_iterator transfer =
      headers.find("transfer-encoding");
  if (transfer != headers.end()) {
    if (transfer->second != "chunked") {
      _state = STATE_ERROR;
      return;
    }
    _chunk_size = 0;
    _chunk_read = 0;
    _chunk_size_buffer.clear();
    _storage_buffer.clear();
    _next_state = STATE_CHUNK_SIZE;
    _state = STATE_HEADERS_COMPLETE;
    return;
  }

  std::map<std::string, std::string>::const_iterator it =
      headers.find("content-length");
  if (it == headers.end()) {
    _next_state = STATE_COMPLETE;
    _state = STATE_HEADERS_COMPLETE;
    return;
  }

  std::string val = it->second;
  if (val.empty()) {
    _next_state = STATE_COMPLETE;
    _state = STATE_HEADERS_COMPLETE;
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
    _next_state = STATE_COMPLETE;
    _state = STATE_HEADERS_COMPLETE;
  } else {
    _content_length = len;
    _storage_buffer.clear();
    _storage_buffer.reserve(_content_length);
    _request.reserve_body(_content_length);
    _next_state = STATE_BODY;
    _state = STATE_HEADERS_COMPLETE;
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

void RequestParser::_handle_state_chunk_size(char c) {
  if (c == '\r') {
    if (_chunk_size_buffer.empty()) {
      _state = STATE_ERROR;
      return;
    }

    std::stringstream ss;
    ss << std::hex << _chunk_size_buffer;

    if (!(ss >> _chunk_size)) {
      _state = STATE_ERROR;
      return;
    }

    _expect_newline = true;
    return;
  }

  if (!std::isxdigit(static_cast<unsigned char>(c))) {
    _state = STATE_ERROR;
    return;
  }

  _chunk_size_buffer += c;
}

void RequestParser::_handle_state_chunk_data(char c) {
  _storage_buffer += c;
  ++_chunk_read;

  if (_chunk_read == _chunk_size) _state = STATE_CHUNK_CRLF;
}

void RequestParser::_handle_state_chunk_crlf(char c) {
  if (c != '\r') {
    _state = STATE_ERROR;
    return;
  }

  _expect_newline = true;
}

void RequestParser::_handle_state_chunk_end(char c) {
  if (c != '\r') {
    _state = STATE_ERROR;
    return;
  }

  _expect_newline = true;
}
