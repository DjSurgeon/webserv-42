
1. **Ingress:** `poll()` detects `POLLIN` on a `ClientSocket`. Member A reads the bytes and appends them to the client's internal string buffer (`_read_buffer`).
2. **Processing:** Member B triggers the `RequestParser`, passing the buffer character by character into the FSM via `.feed(char)`.
3. **Routing:** Once the parser state shifts to `STATE_COMPLETE`, Member C's routing system matches the request's `Host` header and `URI` against the `ServerConfig` matrix using the *Longest Prefix Match* algorithm.
4. **Egress:** The specific handler (Static, CGI, or Error) builds an HTTP response string. Member B packages it into a `Response` object, and Member A pushes it into the client's `_write_buffer` to be safely sent via `POLLOUT`.

---

## 📜 3. Class Component Contracts (The APIs)

To ensure smooth compilation when merging branches, the following class boundaries are strictly enforced:

### A. Network Module (`src/network/`)
- **`ListeningSocket`**: Monitors system ports.
  - *Public API:*
    - `void init(int port);` -> Binds and listens.
    - `int accept_connection();` -> Spawns a raw client descriptor.
- **`ClientSocket`**: Encapsulates active client state.
  - *Public API:*
    - `int getFd() const;`
    - `std::string& getReadBuffer();`
    - `std::string& getWriteBuffer();`

### B. HTTP Parser Module (`src/http/`)
- **`HttpRequest`**: Immutable data container filled by the parser.
  - *Public API:*
    - `const std::string& get_method() const;`
    - `const std::string& get_uri() const;`
    - `const std::map<std::string, std::string>& get_headers() const;`
- **`RequestParser`**: FSM character processor.
  - *Public API:*
    - `e_parser_state feed(char c);` -> Moves state and fills `HttpRequest`.

### C. Configuration Module (`src/config/`)
- **`Context`**: Base shared directive storage.
- **`ServerConfig`** / **`LocationConfig`**: Hierarchical rule storage.
  - *Public API:*
    - `const LocationConfig* find_location(const std::string& uri) const;` -> Implements prefix matching.
