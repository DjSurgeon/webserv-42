// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_CONFIG_LOCATIONCONFIG_HPP_
#define SRC_CONFIG_LOCATIONCONFIG_HPP_

#include <string>
#include <vector>

#include "config/Context.hpp"

class LocationConfig : public Context {
 public:
  // Orthodox Canonical Form
  LocationConfig();
  LocationConfig(const LocationConfig& other);
  LocationConfig& operator=(const LocationConfig& other);
  ~LocationConfig();

  // Getters
  const std::string& get_path() const;
  const std::string& get_cgi_path() const;
  const std::string& get_redirect() const;

  // Setters
  void set_path(const std::string& path);
  void set_cgi_path(const std::string& cgi_path);
  void set_redirect(const std::string& redirect);

 private:
  std::string _path;
  std::string _cgi_path;
  std::string _redirect;
};

#endif  // SRC_CONFIG_LOCATIONCONFIG_HPP_
