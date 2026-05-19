*This project has been created as part of the 42 curriculum by <login1>[, <login2>].*

## 📝 Description
This project is a fully functional, asynchronous, and non-blocking HTTP/1.1 Web Server implemented from scratch in strict C++98. The server is capable of monitoring multiple client connections simultaneously using a single I/O multiplexing kernel call (`poll` or equivalent). It is designed to be resilient, highly performant under stress, and fully compliant with the core constraints of standard web browsers.

### Current Architectural Status
The project has successfully completed its core infrastructure foundation across the following layers:
1. **Network Infrastructure (Sockets Layer):**
   - `ListeningSocket`: A pure RAII engine that safely manages server setup lifecycle (`socket`, `setsockopt`, `bind`, `listen`) preventing kernel descriptor leaks.
   - `ClientSocket`: Manages incoming active connections, automatically enforcing `O_NONBLOCK` settings at birth. It utilizes a zero-copy performance layout where reading modules consume data directly via constant string references (`const std::string&`), mitigating unnecessary heap allocations.

2. **HTTP Core Layer (Parsing Engine):**
   - `HttpRequest`: A passive data structure container (DTO) featuring fully automated case-insensitive header normalization to safeguard the system against capitalization protocol conflicts.
   - `RequestParser`: A rigorous Finite State Machine (FSM) stream parser that ingests incoming TCP data character-by-character. To minimize cyclomatic complexity, the processing switch-case is heavily modularized, delegating each FSM state into isolated private member handlers with extreme protection against buffer overflow and protocol violation attacks.

---

## 💻 Instructions

### Compilation
The project compiles under strict standard verification using `c++` and mandatory school flags.

To compile the primary server architecture:
```bash
make
```

To compile the integration and robustness stress test suite:
```bash
c++ -Wall -Wextra -Werror -std=c++98 -Isrc \
    src/network/ClientSocket.cpp \
    src/http/HttpRequest.cpp \
    src/http/RequestParser.cpp \
    tests/test_parser_stress.cpp -o stress_runner
```

### Execution

The Primary server expects a layout configuration file as its sole runtime argument:

```bash
./webserv [path_to_config.conf]
```

To launch the isolated engine stress test battery:
```bash
./stress_runner
```

---

## 📚 Resources

* **RFC 7230:** Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing.
* **Unix Network Programming:** Comprehensive guide to socket lifecycles and non-blocking multiplexing architectures.

### AI Usage Disclosure

In accordance with the 42 curriculum framework, an AI assistant was utilized as a peer-collaborator to optimize productivity and enforce architectural code resilience. AI technologies were strategically deployed for the following tasks:

* **Architectural Peer Reviewing:** Validating potential file descriptor and memory leaks within the early RAII socket handlers.
* **FSM Cyclomatic Optimization:** Refactoring the massive flat switch-case statement inside `RequestParser::feed` into modular private state handlers to enforce Clean Code standards.
* **Defensive Test Engineering:** Designing specific security attack vectors (such as malformed alphanumeric `Content-Length` layouts and version overruns) to test the robustness of the stream parser before linking against live kernel network boundaries.
