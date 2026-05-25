// Copyright 2026 serjimen vja-nie dlesieur
#include "config/Context.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

Context::Context()
    : _root(""),
      _client_max_body_size(1048576),  // 1MB default
      _autoindex(false) {}

Context::Context(const Context& other)
    : _root(other._root),
      _index_files(other._index_files),
      _error_pages(other._error_pages),
      _allowed_methods(other._allowed_methods),
      _client_max_body_size(other._client_max_body_size),
      _autoindex(other._autoindex) {}

Context& Context::operator=(const Context& other) {
  if (this != &other) {
    _root = other._root;
    _index_files = other._index_files;
    _error_pages = other._error_pages;
    _allowed_methods = other._allowed_methods;
    _client_max_body_size = other._client_max_body_size;
    _autoindex = other._autoindex;
  }
  return *this;
}

Context::~Context() {}

const std::string& Context::get_root() const {
  return _root;
}

const std::vector<std::string>& Context::get_index_files() const {
  return _index_files;
}

const std::map<int, std::string>& Context::get_error_pages() const {
  return _error_pages;
}

const std::vector<std::string>& Context::get_allowed_methods() const {
  return _allowed_methods;
}

size_t Context::get_client_max_body_size() const {
  return _client_max_body_size;
}

bool Context::get_autoindex() const {
  return _autoindex;
}

void Context::set_root(const std::string& root) {
  _root = root;
}

void Context::set_index_files(const std::vector<std::string>& index_files) {
  _index_files = index_files;
}

void Context::add_index_file(const std::string& index_file) {
  _index_files.push_back(index_file);
}

void Context::set_error_pages(const std::map<int, std::string>& error_pages) {
  _error_pages = error_pages;
}

void Context::add_error_page(int code, const std::string& uri) {
  _error_pages[code] = uri;
}

void Context::set_allowed_methods(const std::vector<std::string>& allowed_methods) {
  _allowed_methods = allowed_methods;
}

void Context::add_allowed_method(const std::string& allowed_method) {
  _allowed_methods.push_back(allowed_method);
}

void Context::set_client_max_body_size(size_t size) {
  _client_max_body_size = size;
}

void Context::set_autoindex(bool autoindex) {
  _autoindex = autoindex;
}
