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

 private:
  friend void test_get_mime_type();

  struct MimeMap {
    const char* ext;
    const char* type;
  };

  bool _validate_file_access(const std::string& path, HttpResponse& res);
  std::string _get_mime_type(const std::string& path);
};

#endif  // SRC_HANDLERS_FILEHANDLER_HPP_
