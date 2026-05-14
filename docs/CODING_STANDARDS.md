# ⚠️ C++98 Error Handling & Resource Allocation Standards

This document enforces strict memory management and exception rules to guarantee 0 leaks and 0 crashes across all modules.

---

## 🚫 1. Memory Ownership Rules (The "Who Deletes?" Rule)

To prevent double-free errors or memory leaks, we establish strict object ownership:
* **Heap Allocations:** The use of `new` must be strictly minimized. Prefer stack allocation whenever possible.
* **Socket Ownership:** The `EventLoop` manager owns the lifecycle of `ClientSocket` pointers. No other class is allowed to execute `delete` on a socket pointer.
* **Request Ownership:** The `ClientSocket` owns its associated `HttpRequest` and `RequestParser` instances. They are destroyed automatically when the client disconnects.

---

## 🎛️ 2. Exception Handling Policy

* **Constructors Only:** Exceptions (`throw`) must be used primarily during initialization phases (e.g., `ListeningSocket` fails to bind, `ConfigParser` encounters a syntax error).
* **No Throwing in the Event Loop:** The main runtime loop must **never** crash. If a client sends a malformed request, the parser must transition to `STATE_ERROR` or return an error status code, rather than throwing an exception that could take down the whole server.
* **Catching Strategy:** Every `throw` must have a clearly defined `try-catch` block in the immediate upper layer. Uncaught exceptions that reach the kernel are considered an automatic failure.

---

## 🛑 3. Authorized System Functions Guardrail

We strictly adhere to the 42 subject allowlist. Any usage of unauthorized external functions will block the Pull Request.
* **Allowed for Network:** `socket`, `setsockopt`, `bind`, `listen`, `accept`, `select`/`poll`.
* **Allowed for I/O & Process:** `read`, `write`, `fcntl`, `fork`, `execve`, `pipe`, `dup`, `dup2`.
* **Prohibited:** Any C++11 keyword or container (`auto`, `nullptr`, `std::array`, `shared_ptr`). Use `NULL` for pointers.
