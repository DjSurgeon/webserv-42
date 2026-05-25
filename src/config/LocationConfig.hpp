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
  const std::vector<std::string>& get_cgi_extensions() const;

  // Setters
  void set_path(const std::string& path);
  void set_cgi_path(const std::string& cgi_path);
  void set_redirect(const std::string& redirect);
  void add_cgi_extension(const std::string& ext);

 private:
  std::string _path;
  std::string _cgi_path;
  std::string _redirect;
  std::vector<std::string> _cgi_extensions;
};

#endif  // SRC_CONFIG_LOCATIONCONFIG_HPP_
