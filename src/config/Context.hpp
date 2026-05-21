// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_CONFIG_CONTEXT_HPP_
#define SRC_CONFIG_CONTEXT_HPP_

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class Context {
 public:
  // Orthodox Canonical Form
  Context();
  Context(const Context& other);
  Context& operator=(const Context& other);
  virtual ~Context();

  // Getters
  const std::string& get_root() const;
  const std::vector<std::string>& get_index_files() const;
  const std::map<int, std::string>& get_error_pages() const;
  size_t get_client_max_body_size() const;
  bool get_autoindex() const;

  // Setters
  void set_root(const std::string& root);
  void set_index_files(const std::vector<std::string>& index_files);
  void add_index_file(const std::string& index_file);
  void set_error_pages(const std::map<int, std::string>& error_pages);
  void add_error_page(int code, const std::string& uri);
  void set_client_max_body_size(size_t size);
  void set_autoindex(bool autoindex);

 private:
  std::string _root;
  std::vector<std::string> _index_files;
  std::map<int, std::string> _error_pages;
  size_t _client_max_body_size;
  bool _autoindex;
};

#endif  // SRC_CONFIG_CONTEXT_HPP_
