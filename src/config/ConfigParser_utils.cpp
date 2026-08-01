// Copyright 2026 raperez- serjimen
#include <cctype>
#include <string>
#include <vector>

#include "config/ConfigParser.hpp"

void ConfigParser::trimWhitespace(std::string* line) {
  if (!line) {
    return;
  }
  const std::string whitespace = " \t\r\n\v\f";
  size_t start = line->find_first_not_of(whitespace);

  if (start == std::string::npos) {
    line->clear();
    return;
  }

  size_t end = line->find_last_not_of(whitespace);
  *line = line->substr(start, end - start + 1);
}

void ConfigParser::removeComments(std::string* line) {
  if (!line) {
    return;
  }
  size_t pos = line->find('#');
  if (pos != std::string::npos) {
    line->erase(pos);
  }
}

std::vector<std::string> ConfigParser::tokenize(const std::string& line) {
  std::vector<std::string> tokens;
  std::string currentToken;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line[i];

    if (std::isspace(static_cast<unsigned char>(c))) {
      if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken.clear();
      }
    } else if (c == ';' || c == '{' || c == '}') {
      if (!currentToken.empty()) {
        tokens.push_back(currentToken);
        currentToken.clear();
      }
      tokens.push_back(std::string(1, c));
    } else {
      currentToken += c;
    }
  }

  if (!currentToken.empty()) {
    tokens.push_back(currentToken);
  }

  return tokens;
}
