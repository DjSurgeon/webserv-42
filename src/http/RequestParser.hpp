#ifndef REQUEST_PARSER_HPP
# define REQUEST_PARSER_HPP

# include "http/HttpRequest.hpp"

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
    STATE_ERROR
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

    e_parser_state      feed(char c);
    e_parser_state      get_state() const;
    const HttpRequest&  get_request() const;
    void                reset();

private:
    e_parser_state      _state;
    HttpRequest         _request;
    bool                _expect_newline;
    std::string         _storage_buffer;
    std::string         _current_header_key;
    size_t              _content_length;

    // --- Private State Handlers (The Refactored Switch Delegation) ---
    void                _handle_state_start(char c);
    void                _handle_state_method(char c);
    void                _handle_state_uri(char c);
    void                _handle_state_version(char c);
    void                _handle_state_header_key(char c);
    void                _handle_state_header_value(char c);
    void                _handle_state_body(char c);
    void                _determine_body_transition();
    void                _handle_global_newline(char c);

    // Prevent copying (C++98 style)
    RequestParser(const RequestParser& other);
    RequestParser& operator=(const RequestParser& other);
};

#endif
