// Copyright 2026 serjimen vja-nie dlesieur
#include "config/LocationConfig.hpp"

#include <string>
#include <vector>

LocationConfig::LocationConfig()
    : Context(), _path(""), _cgiPath(""), _redirect("") {}

LocationConfig::LocationConfig(const LocationConfig& other)
    : Context(other),
      _path(other._path),
      _cgiPath(other._cgiPath),
      _redirect(other._redirect),
      _cgiExtensions(other._cgiExtensions) {}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
  if (this != &other) {
    Context::operator=(other);
    _path = other._path;
    _cgiPath = other._cgiPath;
    _redirect = other._redirect;
    _cgiExtensions = other._cgiExtensions;
  }
  return *this;
}

LocationConfig::~LocationConfig() {}

const std::string& LocationConfig::getPath() const {
  return _path;
}

const std::string& LocationConfig::getCgiPath() const {
  return _cgiPath;
}

const std::string& LocationConfig::getRedirect() const {
  return _redirect;
}

const std::vector<std::string>& LocationConfig::getCgiExtensions() const {
  return _cgiExtensions;
}

void LocationConfig::setPath(const std::string& path) {
  _path = path;
}

void LocationConfig::setCgiPath(const std::string& cgiPath) {
  _cgiPath = cgiPath;
}

void LocationConfig::setRedirect(const std::string& redirect) {
  _redirect = redirect;
}

void LocationConfig::addCgiExtension(const std::string& ext) {
  _cgiExtensions.push_back(ext);
}
