// Copyright 2026 serjimen vja-nie dlesieur
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "config/ConfigParser.hpp"

void ConfigParser::parseHostPort(const std::string& listenVal,
                                 std::string* host, std::string* portStr) {
  if (!host || !portStr) {
    return;
  }
  size_t colonPos = listenVal.find(':');
  if (colonPos != std::string::npos) {
    *host = listenVal.substr(0, colonPos);
    *portStr = listenVal.substr(colonPos + 1);
  } else {
    *host = "127.0.0.1";
    *portStr = listenVal;
  }
}

int ConfigParser::validatePort(const std::string& portStr) {
  if (portStr.empty()) {
    throw std::runtime_error(
        "Syntax error: port missing in 'listen' directive");
  }
  for (size_t j = 0; j < portStr.length(); ++j) {
    if (!std::isdigit(static_cast<unsigned char>(portStr[j]))) {
      throw std::runtime_error("Syntax error: invalid port '" + portStr +
                               "' (must be digits)");
    }
  }
  long portVal = std::strtol(portStr.c_str(), NULL, 10);  // NOLINT
  if (portVal < 1 || portVal > 65535) {
    throw std::runtime_error("Syntax error: port '" + portStr +
                             "' out of range (1-65535)");
  }
  return static_cast<int>(portVal);
}
