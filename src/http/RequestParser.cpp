// Copyright 2026 serjimen vja-nie dlesieur
#include "http/RequestParser.hpp"

#include <map>
#include <sstream>
#include <string>

/**
 * @brief Construct a new RequestParser object,
 * initializing FSM state and flag.
 */
RequestParser::RequestParser()
    : _state(STATE_START), _expect_newline(false), _content_length(0) {}

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
    case STATE_COMPLETE:
      break;
    case STATE_ERROR:
      break;
  }

  return _state;
}

/**
 * @brief Handles transitions for CR and LF
 * sequence validation.
 *
 * @param c The active character being evaluated.
 */
void RequestParser::_handle_global_newline(char c) {
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
}
