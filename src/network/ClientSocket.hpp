// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_NETWORK_CLIENTSOCKET_HPP_
#define SRC_NETWORK_CLIENTSOCKET_HPP_

#include <cstddef>
#include <string>

/**
 * @brief RAII class managing an active client connection (The Waiter).
 *
 * Implements architectural: ultra-efficient zero-copy data reading
 * via constant references, combined with strictly controlled mutator methods
 * to encapsulate buffer state and prevent external data corruption.
 * Explicit constructor prevents dangerous implicit conversions from raw FDs.
 */
class ClientSocket {
 public:
  explicit ClientSocket(int client_fd);
  ~ClientSocket();

  // --- Getters (Zero-copy, read-only constant references) ---
  int get_fd() const;
  const std::string& get_read_buffer() const;
  const std::string& get_write_buffer() const;

  // --- Mutators (Controlled state modification gates) ---
  void append_to_read_buffer(const std::string& data);
  void append_to_write_buffer(const std::string& data);
  void consume_read_buffer(size_t bytes);
  void consume_write_buffer(size_t bytes);
  void clear_write_buffer();

  // --- Connection Lifecycle ---
  void set_should_close(bool close);
  bool get_should_close() const;

 private:
  int _fd;
  std::string _read_buffer;
  std::string _write_buffer;
  bool _should_close;

  // Prevent copying (Strict C++98 compliance rule against double-close bugs)
  ClientSocket(const ClientSocket& other);
  ClientSocket& operator=(const ClientSocket& other);
};

#endif  // SRC_NETWORK_CLIENTSOCKET_HPP_
