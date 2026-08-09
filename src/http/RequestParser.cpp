// Copyright 2026 raperez- serjimen
#include "http/RequestParser.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <string>

/**
 * @brief Construct a new RequestParser object,
 * initializing FSM state and flag.
 */
RequestParser::RequestParser()
    : _state(STATE_START),
      _expect_newline(false),
      _content_length(0),
      _header_bytes_read(0),
      _next_state(STATE_START) {}

/**
 * @brief Destroy the RequestParser object.
 */
RequestParser::~RequestParser() {}

/**
 * @brief Feeds a character to the finite state machine parser.
 *
 * @param c The character received from the socket buffer.
 * @return The updated state of the parser.
 */
e_parser_state RequestParser::feed(char c) {
  if (_expect_newline) {
    _handle_global_newline(c);
    return _state;
  }

  switch (_state) {
    case STATE_START:
      _handle_state_start(c);
      break;
    case STATE_METHOD:
      _handle_state_method(c);
      break;
    case STATE_URI:
      _handle_state_uri(c);
      break;
    case STATE_VERSION:
      _handle_state_version(c);
      break;
    case STATE_HEADER_KEY:
      _handle_state_header_key(c);
      break;
    case STATE_HEADER_VALUE:
      _handle_state_header_value(c);
      break;
    case STATE_BODY:
      _handle_state_body(c);
      break;
    case STATE_CHUNK_SIZE:
      _handle_state_chunk_size(c);
      break;
    case STATE_CHUNK_DATA:
      _handle_state_chunk_data(c);
      break;
    case STATE_CHUNK_CRLF:
      _handle_state_chunk_crlf(c);
      break;
    case STATE_CHUNK_END:
      _handle_state_chunk_end(c);
      break;
    case STATE_HEADERS_COMPLETE:
      break;
    case STATE_COMPLETE:
      break;
    case STATE_ERROR:
      break;
  }

  return _state;
}

/**
 * @brief Feeds a block of data to the parser, optimizing for STATE_BODY.
 *
 * @param data The raw data buffer.
 * @param length The length of the raw data.
 * @param consumed Reference to store the number of bytes consumed.
 * @return The updated state of the parser.
 */
e_parser_state RequestParser::feed_buffer(const char* data, size_t length,
                                          size_t& consumed) {
  consumed = 0;
  while (consumed < length) {
    if (_state == STATE_BODY) {
      size_t remaining = _content_length - _storage_buffer.size();
      size_t chunk = std::min(remaining, length - consumed);
      _storage_buffer.append(data + consumed, chunk);
      consumed += chunk;

      if (_storage_buffer.size() == _content_length) {
        _request.swap_body(_storage_buffer);
        _state = STATE_COMPLETE;
        return _state;
      }
    } else {
      feed(data[consumed]);
      _header_bytes_read++;
      if (_state < STATE_HEADERS_COMPLETE && _header_bytes_read > 8192) {
        _state = STATE_ERROR;
      }
      consumed++;
      if (_state == STATE_COMPLETE || _state == STATE_ERROR ||
          _state == STATE_HEADERS_COMPLETE) {
        return _state;
      }
    }
  }
  return _state;
}

/**
 * @brief Handles transitions for CR and LF
 * sequence validation.
 *
 * @param c The active character being evaluated.
 */
/*void RequestParser::_handle_global_newline(char c) {
  if (c == '\n') {
    _expect_newline = false;
    if (_state == STATE_VERSION || _state == STATE_HEADER_VALUE) {
      _state = STATE_HEADER_KEY;
    } else if (_state == STATE_HEADER_KEY) {
      _determine_body_transition();
    }
  } else {
    _state = STATE_ERROR;
  }
}*/

void RequestParser::_handle_global_newline(char c) {
  if (c != '\n') {
    _state = STATE_ERROR;
    return;
  }

  _expect_newline = false;

  switch (_state) {
    case STATE_VERSION:
      _state = STATE_HEADER_KEY;
      break;

    case STATE_HEADER_VALUE:
      _state = STATE_HEADER_KEY;
      break;

    case STATE_HEADER_KEY:
      _determine_body_transition();
      break;

    case STATE_CHUNK_SIZE:
      if (_chunk_size == 0) {
        _state = STATE_CHUNK_END;
      } else {
        _chunk_read = 0;
        _state = STATE_CHUNK_DATA;
      }
      break;

    case STATE_CHUNK_CRLF:
      _chunk_size_buffer.clear();
      _state = STATE_CHUNK_SIZE;
      break;

    case STATE_CHUNK_END:
      _request.set_body(_storage_buffer);
      _storage_buffer.clear();
      _state = STATE_COMPLETE;
      break;

    default:
      break;
  }
}

/**
 * @brief Retrieves the current state of the parser.
 *
 * @return The active state enumeration.
 */
e_parser_state RequestParser::get_state() const {
  return _state;
}

/**
 * @brief Retrieves a read-only constant reference to the parsed HTTP request.
 *
 * @return A constant reference to the HttpRequest object.
 */
const HttpRequest& RequestParser::get_request() const {
  return _request;
}

/**
 * @brief Resets the parser state machine and
 * clears all associated storage buffers.
 */
void RequestParser::reset() {
  _state = STATE_START;
  _request.clear();
  _expect_newline = false;
  _storage_buffer.clear();
  _current_header_key.clear();
  _content_length = 0;
  _header_bytes_read = 0;
  _next_state = STATE_START;
}

void RequestParser::resume_body_parsing() {
  if (_state == STATE_HEADERS_COMPLETE) {
    _state = _next_state;
  }
}
