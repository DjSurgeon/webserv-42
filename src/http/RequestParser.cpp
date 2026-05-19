#include "http/RequestParser.hpp"

RequestParser::RequestParser() : _state(STATE_START), _expect_newline(false) {}

RequestParser::~RequestParser() {}

static bool is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool is_uri_char(char c) {
    return c > 32 && c < 127;
}

e_parser_state RequestParser::feed(char c) {
    switch (_state) {
        case STATE_START:
            if (c == ' ' || c == '\r' || c == '\n') {
                // Skip leading spaces or empty line transitions
                break;
            } else if (is_alpha(c)) {
                std::string method;
                method += c;
                _request.set_method(method);
                _state = STATE_METHOD;
            } else {
                _state = STATE_ERROR;
            }
            break;

        case STATE_METHOD:
            if (is_alpha(c)) {
                _request.set_method(_request.get_method() + c);
            } else if (c == ' ') {
                _state = STATE_URI;
            } else {
                _state = STATE_ERROR;
            }
            break;

        case STATE_URI:
            if (c == ' ') {
                if (_request.get_uri().empty()) {
                    _state = STATE_ERROR;
                } else {
                    _state = STATE_VERSION;
                }
            } else if (is_uri_char(c)) {
                _request.set_uri(_request.get_uri() + c);
            } else {
                _state = STATE_ERROR;
            }
            break;
        case STATE_VERSION:
            if (_expect_newline) {
                if (c == '\n') {
                    _expect_newline = false;
                    _state = STATE_HEADER_KEY;
                } else {
                    _state = STATE_ERROR;
                }
            } else {
                if (c == '\r') {
                    if (_request.get_version() == "HTTP/1.1") {
                        _expect_newline = true;
                    } else {
                        _state = STATE_ERROR;
                    }
                } else {
                    size_t len = _request.get_version().size();
                    if (len < 8 && c == "HTTP/1.1"[len]) {
                        _request.set_version(_request.get_version() + c);
                    } else {
                        _state = STATE_ERROR;
                    }
                }
            }
            break;
        case STATE_HEADER_KEY:
            break;
        case STATE_HEADER_VALUE:
            break;
        case STATE_BODY:
            break;
        case STATE_COMPLETE:
            break;
        case STATE_ERROR:
            break;
    }

    return _state;
}

e_parser_state RequestParser::get_state() const {
    return _state;
}

const HttpRequest& RequestParser::get_request() const {
    return _request;
}

void RequestParser::reset() {
    _state = STATE_START;
    _request.clear();
    _expect_newline = false;
}
