# 🗺️ Webserv Architecture & Data Flow Blueprint

This document defines the official architecture, class interfaces, and data flow contracts for `webserv-42`. All team members must adhere to these structural specifications to prevent integration conflicts.

---

## 🧩 1. High-Level Macro Architecture

The server is divided into three distinct decoupled layers, each managed by a specific team member:

1. **The Network Layer (Member A):** Manages non-blocking I/O multiplexing (`poll`), raw socket connections, and system event triggers.
2. **The Parsing Layer (Member B):** Converts raw character streams from client sockets into validated HTTP Request objects using a Finite State Machine (FSM).
3. **The Configuration & Logic Layer (Member C):** Holds NGINX-style rules, matches incoming requests to virtual hosts/locations, and handles CGI process execution.

---

## 🔄 2. Complete Request-Response Lifecycle (The Data Flow)

```
[Browser Client]
│ (Raw TCP Data)
▼
[ClientSocket] ──(append to _read_buffer)──► [RequestParser]
│ (.feed(char))
▼
[ListeningLoop] ◄──(evaluates rules)── [ServerConfig / LocationConfig]
│
▼
[ResponseBuilder] ──(string stream)──► [ClientSocket._write_buffer]
│
▼
[Browser Client]
```

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
    - `int get_fd() const;`
    - `std::string& get_read_buffer();`
    - `std::string& get_write_buffer();`

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
