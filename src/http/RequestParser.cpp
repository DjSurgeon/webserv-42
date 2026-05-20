// Copyright 2026 serjimen vja-nie dlesieur
#include "http/RequestParser.hpp"
#include <map>
#include <sstream>
#include <string>

/**
 * @brief Construct a new RequestParser object, initializing FSM state and flag.
 */
RequestParser::RequestParser() : _state(STATE_START), _expect_newline(false), _content_length(0) {}

/**
 * @brief Destroy the RequestParser object.
 */
RequestParser::~RequestParser() {}

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
 * @brief Helper utility to check if a character is a valid non-space URI character.
 *
 * @param c The character to check.
 * @return True if character is printable visible ASCII and non-space, false otherwise.
 */
static bool is_uri_char(char c) {
    return c > 32 && c < 127;
}

/**
 * @brief Helper utility to check if a character is a valid header key token character.
 *
 * @param c The character to check.
 * @return True if character is alphanumeric or a hyphen, false otherwise.
 */
static bool is_header_key_char(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
}

/**
 * @brief Helper utility to trim spaces and tabs from the beginning and end of a string.
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
        case STATE_START:        _handle_state_start(c); break;
        case STATE_METHOD:       _handle_state_method(c); break;
        case STATE_URI:          _handle_state_uri(c); break;
        case STATE_VERSION:      _handle_state_version(c); break;
        case STATE_HEADER_KEY:   _handle_state_header_key(c); break;
        case STATE_HEADER_VALUE: _handle_state_header_value(c); break;
        case STATE_BODY:         _handle_state_body(c); break;
        case STATE_COMPLETE:     break;
        case STATE_ERROR:        break;
    }

    return _state;
}

/**
 * @brief Handles transitions for carriage return and newline sequence validation.
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
 * @brief Evaluates start state characters, discarding leading spaces and line feeds.
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

/**
 * @brief Parses individual characters of a header name, transitioning to value on colon.
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
 * @brief Parses individual characters of a header value, trimming surrounding space and tabs.
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
    } else if (c == ' ' || c == '\t' || (c >= 33 && c <= 126) || (unsigned char)c >= 128) {
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
 * @brief Decides transition to STATE_BODY or STATE_COMPLETE based on Content-Length.
 */
void RequestParser::_determine_body_transition() {
    const std::map<std::string, std::string>& headers = _request.get_headers();
    std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
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
 * @brief Resets the parser state machine and clears all associated storage buffers.
 */
void RequestParser::reset() {
    _state = STATE_START;
    _request.clear();
    _expect_newline = false;
    _storage_buffer.clear();
    _current_header_key.clear();
    _content_length = 0;
}
