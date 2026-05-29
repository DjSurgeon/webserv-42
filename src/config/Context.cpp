// Copyright 2026 serjimen vja-nie dlesieur
#include "config/Context.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

Context::Context()
    : _root(""),
      _clientMaxBodySize(1048576),  // 1MB default
      _autoindex(false) {}

Context::Context(const Context& other)
    : _root(other._root),
      _indexFiles(other._indexFiles),
      _errorPages(other._errorPages),
      _allowedMethods(other._allowedMethods),
      _clientMaxBodySize(other._clientMaxBodySize),
      _autoindex(other._autoindex) {}

Context& Context::operator=(const Context& other) {
  if (this != &other) {
    _root = other._root;
    _indexFiles = other._indexFiles;
    _errorPages = other._errorPages;
    _allowedMethods = other._allowedMethods;
    _clientMaxBodySize = other._clientMaxBodySize;
    _autoindex = other._autoindex;
  }
  return *this;
}

Context::~Context() {}

const std::string& Context::getRoot() const {
  return _root;
}

const std::vector<std::string>& Context::getIndexFiles() const {
  return _indexFiles;
}

const std::map<int, std::string>& Context::getErrorPages() const {
  return _errorPages;
}

const std::vector<std::string>& Context::getAllowedMethods() const {
  return _allowedMethods;
}

size_t Context::getClientMaxBodySize() const {
  return _clientMaxBodySize;
}

bool Context::getAutoindex() const {
  return _autoindex;
}

void Context::setRoot(const std::string& root) {
  _root = root;
}

void Context::setIndexFiles(const std::vector<std::string>& indexFiles) {
  _indexFiles = indexFiles;
}

void Context::addIndexFile(const std::string& indexFile) {
  _indexFiles.push_back(indexFile);
}

void Context::setErrorPages(const std::map<int, std::string>& errorPages) {
  _errorPages = errorPages;
}

void Context::addErrorPage(int code, const std::string& uri) {
  _errorPages[code] = uri;
}

void Context::setAllowedMethods(
    const std::vector<std::string>& allowedMethods) {
  _allowedMethods = allowedMethods;
}

void Context::addAllowedMethod(const std::string& allowedMethod) {
  _allowedMethods.push_back(allowedMethod);
}

void Context::setClientMaxBodySize(size_t size) {
  _clientMaxBodySize = size;
}

void Context::setAutoindex(bool autoindex) {
  _autoindex = autoindex;
}
