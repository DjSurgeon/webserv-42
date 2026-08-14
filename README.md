*This project has been created as part of the 42 curriculum by raperez-, serjimen.*

## 📝 Description
This project is a fully functional, asynchronous, and non-blocking HTTP/1.1 Web Server implemented from scratch in strict C++98. The server is capable of monitoring multiple client connections simultaneously using a single I/O multiplexing kernel call (`poll` or equivalent). It is designed to be resilient, highly performant under stress, and fully compliant with the core constraints of standard web browsers.

### Current Architectural Status
The project has successfully completed its core infrastructure foundation across the following layers:

1. **Canonical C++ Layout & SRP Refactoring:**
   - **Industry Standard Structure**: The project strictly separates public interfaces (`include/`) from private implementations (`src/`), ensuring strong encapsulation, clean semantic `#include` directives, and zero compilation warnings.
   - **Partial Implementations**: Monolithic source files (like `HttpResponse.cpp`) were shattered into focused, specialized sub-modules (e.g., `HttpResponse_error.cpp`). Functions were rigorously decoupled following the **Single Responsibility Principle (SRP)**, drastically reducing cognitive load and easing long-term maintainability.

2. **Network Infrastructure (Sockets Layer):**
   - `ListeningSocket`: A pure RAII engine that safely manages server setup lifecycle (`socket`, `setsockopt`, `bind`, `listen`) preventing kernel descriptor leaks.
   - `ClientSocket`: Manages incoming active connections, automatically enforcing `O_NONBLOCK` settings at birth. It utilizes a zero-copy performance layout where reading modules consume data directly via constant string references (`const std::string&`), mitigating unnecessary heap allocations.

2. **HTTP Core Layer (Parsing & Response Engine):**
   - `HttpRequest`: A passive data structure container (DTO) featuring fully automated case-insensitive header normalization to safeguard the system against capitalization protocol conflicts.
   - `RequestParser`: A rigorous Finite State Machine (FSM) stream parser that ingests incoming TCP data character-by-character. To minimize cyclomatic complexity, the processing switch-case is heavily modularized, delegating each FSM state into isolated private member handlers with extreme protection against buffer overflow and protocol violation attacks.
   - `HttpResponse`: Fully compliant HTTP/1.1 response builder. Incorporates a smart internal contingency system that automatically generates robust fallback HTML error pages (400, 403, 404, 405, 500) and injects strictly required headers (Content-Length, Content-Type) without duplicating logic, adhering to the DRY principle via an internal static reason phrase dictionary.

3. **Configuration Architecture (NGINX Style):**
   - A highly modular and strict C++98 hierarchical inheritance tree (`Context` -> `ServerConfig` & `LocationConfig`) established to store layout rules efficiently, entirely refactored to conform to strict `camelCase` coding standards.
   - It guarantees memory-safe deep cloning between configuration contexts leveraging the Orthodox Canonical Form, effectively preparing the runtime to absorb custom `.conf` directives safely.
   - **Bespoke Recursive Descent Parser (`ConfigParser`)**: Ingests, tokenizes, and structures complex `.conf` layout files in $O(N)$ time complexity. Physically shattered into 5 specialized sub-modules (e.g. `ConfigParser_server.cpp`, `ConfigParser_location.cpp`, `ConfigParser_context.cpp`) to strictly adhere to the Single Responsibility Principle (SRP). It automatically handles comment sanitization, whitespace trimming, and context cascading. It provides extreme resilience against edge cases (missing semicolons, unclosed brackets, unknown directives, etc.) aborting gracefully instead of inducing segmentation faults.

4. **Content Delivery Layer:**
   - `FileHandler`: A secure, non-blocking I/O engine responsible for physical file serving and dynamic autoindexing. It strictly adheres to POSIX standards (`<dirent.h>`, `<sys/stat.h>`, `<unistd.h>`) to authenticate read permissions and sanitize paths before opening files.
   - **Binary Safety & Performance**: Employs `std::ios::binary` and stream buffering (`rdbuf()`) for ultra-fast, zero-corruption reads of media files.
   - **Dynamic Autoindex**: Safely generates navigational HTML indices for directories, preventing dead links via strict URI sanitization, fully bypassing nested "Arrow Code" through single-responsibility helper functions.
   - **Strict DELETE Engine**: Implements the HTTP DELETE method with multiple layers of system-level defensive programming. Validates file existence and write permissions (`W_OK`), strictly denies recursive directory wipes, and elegantly handles hardware race-conditions. It correctly signals successful deletions using the exact standard `204 No Content` HTTP format without emitting any payload bytes.

5. **CGI (Common Gateway Interface) Engine:**
   - `CgiHandler`: A fully isolated IPC-driven module capable of forking external scripts dynamically (e.g., Python, PHP) based on location configurations.
   - **Anti-Deadlock Storage**: Safe offloading of massive POST request payloads via `tmpfile` buffer allocations to bypass POSIX kernel pipe-locking capacity constraints.
   - **RFC 3875 Strict Parser**: Implements an NGINX-style rigorous header separation routine (`\r\n\r\n` boundary checks) that aggressively sanitizes untrusted CGI output. Intercepts script pseudo-headers (like `Status`) directly into the HTTP core pipeline instead of erroneously leaking them into payloads, and safely injects an exact byte-calculated `Content-Length`. Any non-compliant execution immediately triggers a safe `502 Bad Gateway` containment boundary.

6. **Routing, Firewall & Event Multiplexing:**
   - `StaticRouter` acting as an intelligent path translation engine and security firewall (instantly generating HTTP 405 for illegal methods and HTTP 413 to repel malicious massive payload attacks via `client_max_body_size`).
   - `EventLoop` routing dynamic/static requests via `cgi_ext` dynamic extension resolution. It also implements an $O(1)$ HTTP 301 Short-circuit redirect, safely bypassing heavy file handlers to deliver immediate redirection instructions.
   - **Context-Aware Error Propagation:** Native support for injecting Context pointers down the stack, enabling handlers to serve completely personalized visual error pages (`error_page` directive) per isolated location.
   - Robust HTTP/1.1 TCP Lifecycle Management: Full Keep-Alive and Pipelining support via `RequestParser::reset()`, including graceful connection closure preventing File Descriptor leakage under heavy load. The infrastructure achieves a proven flawless status under strict `valgrind` tracing (0 memory leaks).

7. **Advanced Bonus Features:**
   - **Native Cookie Management:** Extends the baseline HTTP/1.1 protocol handling to securely parse, trim, and extract browser-sent `Cookie` payloads into queryable C++ Maps during request digestion. It simultaneously features an asynchronous, multi-header emission engine allowing secure injection of independent `Set-Cookie` directives bypassing `std::map` key-collision boundaries.

---

## 💻 Instructions

### Compilation
The project compiles under strict standard verification using `c++` and mandatory school flags.

To compile the primary server architecture:
```bash
make
```

### 🧪 Testing & Quality Assurance
Webserv-42 treats tests as first-class citizens. To ensure rock-solid stability without regressions, we have implemented a strict QA ecosystem within the `tests/` directory:
- **AAA Pattern**: All unit and integration tests strictly adhere to the **Arrange, Act, Assert** design pattern to guarantee clean, readable, and maintainable test code.
- **Coverage Matrix**: A visual heat-map (`tests/TEST_CASES.md`) tracking edge cases, assertions, and current protection status.
- **Automated Suites**: Tests can be launched effortlessly via built-in bash scripts.

To compile and execute the complete test suite:
```bash
make test
# Or launch the comprehensive script:
./tests/run_all_tests.sh
```

### Execution

The Primary server expects a layout configuration file as its sole runtime argument:

```bash
./webserv [path_to_config.conf]
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
