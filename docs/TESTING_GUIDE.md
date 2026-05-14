# 🧪 Sandbox Testing & Local Verification Guide

Before submitting a Pull Request, you must verify your atomic module using these manual testing workflows.

---

## 🌐 1. Testing the Network Layer (Member A)

To verify that your `ListeningSocket` and `ClientSocket` are properly accepting connections without blocking:

1. Compile and run your temporary main: `./webserv`
2. Open a separate terminal and trigger a raw connection using `netcat` (`nc`):
   ```bash
   nc -v localhost 8080
   ```

**Success Criteria:** The server logs must show a new connection accepted with a valid file descriptor, and the server must remain responsive to a second `nc` command in another terminal window.

---

## 🧠 2. Testing the HTTP Parser Stream (Member B)

To verify that the FSM parses fragmented data character by character without losing state:

Create a local test harness that feeds a split string array into the parser:

```cpp
// Inside your local test main:
parser.feed('G'); parser.feed('E'); parser.feed('T'); parser.feed(' ');
// Simulate network delay...
parser.feed('/'); parser.feed('\r'); parser.feed('\n');
```

**Success Criteria:** The parser must gracefully stay in `STATE_URI` or `STATE_VERSION` during fragments, and transition to `STATE_COMPLETE` only when the exact protocol terminators are fully digested.
