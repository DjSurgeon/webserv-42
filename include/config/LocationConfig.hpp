// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_CONFIG_LOCATIONCONFIG_HPP_
#define INCLUDE_CONFIG_LOCATIONCONFIG_HPP_

#include <string>
#include <vector>

#include "config/Context.hpp"

/**
 * @brief Represents a 'location' block inside an NGINX-style configuration.
 *
 * Inherits from Context to support generic directives, while adding
 * specific routing and CGI rules for a URI path.
 */
class LocationConfig : public Context {
 public:
  // Orthodox Canonical Form
  LocationConfig();
  LocationConfig(const LocationConfig& other);
  LocationConfig& operator=(const LocationConfig& other);
  ~LocationConfig();

  // Getters
  const std::string& getPath() const;
  const std::string& getCgiPath() const;
  const std::string& getRedirect() const;
  const std::vector<std::string>& getCgiExtensions() const;
  const std::string& getUploadPath() const;

  // Setters
  void setPath(const std::string& path);
  void setCgiPath(const std::string& cgiPath);
  void setRedirect(const std::string& redirect);
  void addCgiExtension(const std::string& ext);
  void setUploadPath(const std::string& redirect);

 private:
  std::string _path;
  std::string _cgiPath;
  std::string _redirect;
  std::vector<std::string> _cgiExtensions;
  std::string _uploadPath;
};

#endif  // INCLUDE_CONFIG_LOCATIONCONFIG_HPP_
