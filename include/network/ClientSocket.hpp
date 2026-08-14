// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_NETWORK_CLIENTSOCKET_HPP_
#define INCLUDE_NETWORK_CLIENTSOCKET_HPP_

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
  int getFd() const;
  const std::string& getReadBuffer() const;
  const std::string& getWriteBuffer() const;
  const char* getWriteData() const;
  size_t getWriteLength() const;

  // --- Mutators (Controlled state modification gates) ---
  void appendToReadBuffer(const std::string& data);
  void appendToReadBuffer(const char* data, size_t len);
  void appendToWriteBuffer(const std::string& data);
  void swapWriteBuffer(std::string& data);
  void consumeReadBuffer(size_t bytes);
  void consumeWriteBuffer(size_t bytes);
  void clearWriteBuffer();

  // --- Connection Lifecycle ---
  void setShouldClose(bool close);
  bool getShouldClose() const;
  bool isWriteBufferFull() const;

 private:
  int _fd;
  std::string _readBuffer;
  std::string _writeBuffer;
  size_t _writeOffset;
  bool _shouldClose;

  // Prevent copying (Strict C++98 compliance rule against double-close bugs)
  ClientSocket(const ClientSocket& other);
  ClientSocket& operator=(const ClientSocket& other);
};

#endif  // INCLUDE_NETWORK_CLIENTSOCKET_HPP_
