// Copyright 2026 serjimen vja-nie dlesieur
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#define DEFAULT_PORT 8080
#define DEFAULT_COUNT 100

enum e_test_mode {
  MODE_FLOOD,
  MODE_SLOWLORIS,
  MODE_GARBAGE,
  MODE_DROP,
  MODE_DELETE,
  MODE_UNKNOWN
};

struct Connection {
  int fd;
  std::string to_send;
  size_t sent;
  bool completed;
  time_t last_action;
};

e_test_mode parse_mode(const std::string& m) {
  if (m == "flood") return MODE_FLOOD;
  if (m == "slowloris") return MODE_SLOWLORIS;
  if (m == "garbage") return MODE_GARBAGE;
  if (m == "drop") return MODE_DROP;
  if (m == "delete") return MODE_DELETE;
  return MODE_UNKNOWN;
}

int set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return -1;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char** argv) {
  e_test_mode mode = MODE_FLOOD;
  int count = DEFAULT_COUNT;
  int port = DEFAULT_PORT;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--mode" && i + 1 < argc) {
      mode = parse_mode(argv[++i]);
    } else if (arg == "--count" && i + 1 < argc) {
      count = std::atoi(argv[++i]);
    } else if (arg == "--port" && i + 1 < argc) {
      port = std::atoi(argv[++i]);
    }
  }

  if (mode == MODE_UNKNOWN) {
    std::cerr << "Usage: " << argv[0]
              << " [--mode flood|slowloris|garbage|drop|delete] [--count N] "
                 "[--port P]"
              << std::endl;
    return 1;
  }

  std::cout << "Starting stress client: mode=" << mode << ", count=" << count
            << ", port=" << port << std::endl;

  std::vector<Connection> conns;
  std::vector<pollfd> pfds;

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  for (int i = 0; i < count; ++i) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      std::perror("socket");
      continue;
    }
    set_nonblocking(fd);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      if (errno != EINPROGRESS) {
        std::perror("connect");
        close(fd);
        continue;
      }
    }

    Connection c;
    c.fd = fd;
    c.sent = 0;
    c.completed = false;
    c.last_action = time(NULL);

    if (mode == MODE_FLOOD) {
      c.to_send =
          "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection: "
          "close\r\n\r\n";
    } else if (mode == MODE_SLOWLORIS) {
      c.to_send =
          "GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: "
          "Slowloris\r\n";
    } else if (mode == MODE_GARBAGE) {
      c.to_send =
          "INV@LID METHOD / HTTP/1.1\r\nContent-Length: "
          "99999\r\n\r\nGARBAGE_DATA_STREAM_!!!";
    } else if (mode == MODE_DROP) {
      c.to_send = "GET /";
    } else if (mode == MODE_DELETE) {
      c.to_send =
          "DELETE /file_to_delete.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    }

    conns.push_back(c);
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT | POLLIN;
    pfd.revents = 0;
    pfds.push_back(pfd);
  }

  int active = conns.size();
  std::cout << "Successfully opened " << active << " connections." << std::endl;

  while (active > 0) {
    int ret = poll(&pfds[0], pfds.size(), 100);
    if (ret < 0) {
      std::perror("poll");
      break;
    }

    for (size_t i = 0; i < pfds.size(); ++i) {
      if (conns[i].completed || conns[i].fd == -1) continue;

      if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
        close(conns[i].fd);
        conns[i].fd = -1;
        conns[i].completed = true;
        active--;
        continue;
      }

      if (pfds[i].revents & POLLOUT) {
        if (mode == MODE_SLOWLORIS) {
          // Send 1 byte only if enough time passed
          if (time(NULL) - conns[i].last_action >= 1) {
            if (conns[i].sent < conns[i].to_send.length()) {
              send(conns[i].fd, &conns[i].to_send[conns[i].sent], 1, 0);
              conns[i].sent++;
              conns[i].last_action = time(NULL);
            } else {
              // Keep connection open without completing the request
            }
          }
        } else if (mode == MODE_DROP) {
          send(conns[i].fd, conns[i].to_send.c_str(), conns[i].to_send.length(),
               0);
          close(conns[i].fd);
          conns[i].fd = -1;
          conns[i].completed = true;
          active--;
        } else {
          // Flood or Garbage: send as much as possible
          int s = send(conns[i].fd, conns[i].to_send.c_str() + conns[i].sent,
                       conns[i].to_send.length() - conns[i].sent, 0);
          if (s > 0) {
            conns[i].sent += s;
            if (conns[i].sent >= conns[i].to_send.length()) {
              // Sent everything
              if (mode == MODE_GARBAGE) {
                // For garbage, we don't necessarily expect a full response
                // before closing, but we'll wait for POLLIN for a bit.
              }
            }
          }
        }
      }

      if (pfds[i].revents & POLLIN) {
        char buf[4096];
        int r = recv(conns[i].fd, buf, sizeof(buf), 0);
        if (r <= 0) {
          close(conns[i].fd);
          conns[i].fd = -1;
          conns[i].completed = true;
          active--;
        } else {
          // Received some response data
          if (mode == MODE_FLOOD || mode == MODE_GARBAGE ||
              mode == MODE_DELETE) {
            // Mark as done after receiving something
            close(conns[i].fd);
            conns[i].fd = -1;
            conns[i].completed = true;
            active--;
          }
        }
      }

      // Timeout for slowloris (don't stay forever in this test)
      if (mode == MODE_SLOWLORIS && time(NULL) - conns[i].last_action > 5 &&
          conns[i].sent > 10) {
        close(conns[i].fd);
        conns[i].fd = -1;
        conns[i].completed = true;
        active--;
      }
    }
  }

  std::cout << "Stress test finished." << std::endl;
  return 0;
}
