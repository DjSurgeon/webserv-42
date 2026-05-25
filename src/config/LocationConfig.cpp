// Copyright 2026 serjimen vja-nie dlesieur
#include "config/LocationConfig.hpp"

#include <string>
#include <vector>

LocationConfig::LocationConfig()
    : Context(), _path(""), _cgi_path(""), _redirect("") {}

LocationConfig::LocationConfig(const LocationConfig& other)
    : Context(other),
      _path(other._path),
      _cgi_path(other._cgi_path),
      _redirect(other._redirect) {}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
  if (this != &other) {
    Context::operator=(other);
    _path = other._path;
    _cgi_path = other._cgi_path;
    _redirect = other._redirect;
  }
  return *this;
}

LocationConfig::~LocationConfig() {}

const std::string& LocationConfig::get_path() const {
  return _path;
}

const std::string& LocationConfig::get_cgi_path() const {
  return _cgi_path;
}

const std::string& LocationConfig::get_redirect() const {
  return _redirect;
}

void LocationConfig::set_path(const std::string& path) {
  _path = path;
}

void LocationConfig::set_cgi_path(const std::string& cgi_path) {
  _cgi_path = cgi_path;
}

void LocationConfig::set_redirect(const std::string& redirect) {
  _redirect = redirect;
}
