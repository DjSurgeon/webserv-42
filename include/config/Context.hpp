// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_CONFIG_CONTEXT_HPP_
#define INCLUDE_CONFIG_CONTEXT_HPP_

#include <cstddef>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Base class for shared configuration directives (root, error_page, etc.).
 *
 * Provides inheritance-based reuse for parsing directives that can appear
 * at both the server block level and the location block level.
 */
class Context {
 public:
  // Orthodox Canonical Form
  Context();
  Context(const Context& other);
  Context& operator=(const Context& other);
  virtual ~Context();

  // Getters
  const std::string& getRoot() const;
  const std::vector<std::string>& getIndexFiles() const;
  const std::map<int, std::string>& getErrorPages() const;
  const std::vector<std::string>& getAllowedMethods() const;
  size_t getClientMaxBodySize() const;
  bool getAutoindex() const;

  // Setters
  void setRoot(const std::string& root);
  void setIndexFiles(const std::vector<std::string>& indexFiles);
  void addIndexFile(const std::string& indexFile);
  void setErrorPages(const std::map<int, std::string>& errorPages);
  void addErrorPage(int code, const std::string& uri);
  void setAllowedMethods(const std::vector<std::string>& allowedMethods);
  void addAllowedMethod(const std::string& allowedMethod);
  void setClientMaxBodySize(size_t size);
  void setAutoindex(bool autoindex);

 private:
  std::string _root;
  std::vector<std::string> _indexFiles;
  std::map<int, std::string> _errorPages;
  std::vector<std::string> _allowedMethods;
  size_t _clientMaxBodySize;
  bool _autoindex;
};

#endif  // INCLUDE_CONFIG_CONTEXT_HPP_
