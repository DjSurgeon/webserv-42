# ⚠️ Webserv-42 Coding Standards

This document enforces strict coding conventions across the entire codebase.
All rules are validated automatically by **cpplint** in our CI pipeline.

> **Standard:** C++98 · **Linter:** cpplint (Google C++ Style) · **Formatter:** clang-format

---

## 📄 1. Copyright Header

Every `.cpp` and `.hpp` file **must** begin with a copyright line as its very first line.

```cpp
// Copyright 2026 serjimen vja-nie dlesieur
```

- Must be the **first line** of the file, before any `#ifndef` or `#include`.
- cpplint will reject any file missing this header.

---

## 🛡️ 2. Header Guards

Every `.hpp` file must use include guards that match the **full path** of the file,
in uppercase, with slashes replaced by underscores and a **trailing underscore**.

### Format

```cpp
#ifndef SRC_<DIR>_<FILENAME>_HPP_
#define SRC_<DIR>_<FILENAME>_HPP_

// ... content ...

#endif  // SRC_<DIR>_<FILENAME>_HPP_
```

### Examples used in this project

| File | Guard |
|---|---|
| `src/http/HttpRequest.hpp` | `SRC_HTTP_HTTPREQUEST_HPP_` |
| `src/http/RequestParser.hpp` | `SRC_HTTP_REQUESTPARSER_HPP_` |
| `src/network/ListeningSocket.hpp` | `SRC_NETWORK_LISTENINGSOCKET_HPP_` |
| `src/network/ClientSocket.hpp` | `SRC_NETWORK_CLIENTSOCKET_HPP_` |

### Rules

- `#endif` **must** have a trailing comment: `// GUARD_NAME`
- Two spaces before the `//` comment.
- Never use `#pragma once` (not portable, not C++98).

---

## 📦 3. Include Order (Google Style)

Includes must follow this **strict order**, with a **blank line** between each group:

1. **Own header** (the `.hpp` corresponding to this `.cpp`)
2. **C system headers** (`<unistd.h>`, `<sys/socket.h>`, `<fcntl.h>`, etc.)
3. **C++ standard headers** (`<string>`, `<map>`, `<iostream>`, etc.)
4. **Project headers** (other `.hpp` from our codebase)

### Example

```cpp
// Copyright 2026 serjimen vja-nie dlesieur
#include "network/ClientSocket.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <stdexcept>
#include <string>
```

### Rules

- Headers within each group must be sorted **alphabetically**.
- Use `#include` (no space after `#`). Never `# include`.
- Always use the full project-relative path: `"http/HttpRequest.hpp"`, not `"HttpRequest.hpp"`.

---

## 📐 4. Include What You Use

Every file must **explicitly include** the headers for all types it uses,
even if they might be transitively included by another header.

| If you use... | You must include... |
|---|---|
| `std::string` | `<string>` |
| `std::map` | `<map>` |
| `std::vector` | `<vector>` |
| `size_t` | `<cstddef>` |
| `std::cout` / `std::cerr` | `<iostream>` |
| `std::runtime_error` | `<stdexcept>` |
| `std::stringstream` | `<sstream>` |

---

## 🏗️ 5. Class Indentation (Google Style)

Inside a class declaration:

- `public:`, `private:`, `protected:` → **1 space** indent
- Members and methods → **2 spaces** indent
- **Never** use tabs. Always spaces.

### ❌ Incorrect

```cpp
class Foo {
public:
    void bar();
private:
    int _x;
};
```

### ✅ Correct

```cpp
class Foo {
 public:
  void bar();

 private:
  int _x;
};
```

---

## 📏 6. Line Length Limit

**No line may exceed 80 characters.**

### How to break long lines

**After a comma (function parameters):**

```cpp
void add_header(const std::string& key,
                const std::string& value);
```

**Before a logical operator:**

```cpp
if (c == ' ' || c == '\t'
    || (c >= 33 && c <= 126)
    || (unsigned char)c >= 128) {
```

**Constructor initializer lists:**

```cpp
RequestParser::RequestParser()
    : _state(STATE_START),
      _expect_newline(false),
      _content_length(0) {}
```

**Long type declarations:**

```cpp
std::map<std::string, std::string>::const_iterator
    it = headers.find("content-length");
```

**String concatenation (C++98 adjacent literals):**

```cpp
std::string msg = "This is a very long message "
                  "split across lines.";
```

### Exceptions

- URLs in comments may exceed 80 chars.
- Header guard comments on `#endif` may exceed if the path is long.

---

## 🚫 7. Memory Ownership Rules

To prevent double-free errors or memory leaks:

- **Heap Allocations:** Use of `new` must be strictly minimized.
  Prefer stack allocation whenever possible.
- **Socket Ownership:** The `EventLoop` manager owns the lifecycle
  of `ClientSocket` pointers. No other class may `delete` a socket.
- **Request Ownership:** The `ClientSocket` owns its associated
  `HttpRequest` and `RequestParser` instances. They are destroyed
  automatically when the client disconnects.

---

## 🎛️ 8. Exception Handling Policy

- **Constructors Only:** Exceptions (`throw`) must be used primarily
  during initialization phases (e.g., `ListeningSocket` fails to bind,
  `ConfigParser` encounters a syntax error).
- **No Throwing in the Event Loop:** The main runtime loop must
  **never** crash. If a client sends a malformed request, the parser
  must transition to `STATE_ERROR` rather than throwing.
- **Catching Strategy:** Every `throw` must have a clearly defined
  `try-catch` block in the immediate upper layer. Uncaught exceptions
  that reach the kernel are considered an automatic failure.

---

## 🛑 9. Authorized System Functions

We strictly adhere to the 42 subject allowlist.
Any unauthorized function will block the Pull Request.

- **Network:** `socket`, `setsockopt`, `bind`, `listen`, `accept`,
  `select`/`poll`.
- **I/O & Process:** `read`, `write`, `fcntl`, `fork`, `execve`,
  `pipe`, `dup`, `dup2`.
- **Prohibited:** Any C++11 keyword or container (`auto`, `nullptr`,
  `std::array`, `shared_ptr`). Use `NULL` for pointers.

---

## ✅ Quick Checklist (before every commit)

```bash
# Run the linter
cpplint --recursive src/

# Check formatting
clang-format --dry-run --Werror src/**/*.cpp src/**/*.hpp

# Build
make re

# Run tests
make test
```

