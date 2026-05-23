// Copyright 2026 serjimen vja-nie dlesieur
#ifndef SRC_HANDLERS_FILEHANDLER_HPP_
#define SRC_HANDLERS_FILEHANDLER_HPP_

#include <string>

#include "http/HttpResponse.hpp"

class FileHandler {
 public:
  // Orthodox Canonical Form
  FileHandler();
  FileHandler(const FileHandler& other);
  FileHandler& operator=(const FileHandler& other);
  ~FileHandler();

  bool serve_file(const std::string& physical_path, HttpResponse& res);
  bool delete_file(const std::string& physical_path, HttpResponse& res);
  void generate_autoindex(const std::string& dir_path, const std::string& uri,
                          HttpResponse& res);

 private:
  friend void test_get_mime_type();

  struct MimeMap {
    const char* ext;
    const char* type;
  };

  bool _validate_file_access(const std::string& path, HttpResponse& res);
  std::string _get_mime_type(const std::string& path);
  std::string _build_autoindex_html(const std::string& dir_path,
                                    const std::string& uri);
};

#endif  // SRC_HANDLERS_FILEHANDLER_HPP_
