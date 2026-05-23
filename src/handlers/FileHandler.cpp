// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/FileHandler.hpp"

#include <cstddef>
#include <string>

// -----------------------------------------------------------------------------
// Orthodox Canonical Form
// -----------------------------------------------------------------------------

FileHandler::FileHandler() {}

FileHandler::FileHandler(const FileHandler& other) {
  (void)other;
}

FileHandler& FileHandler::operator=(const FileHandler& other) {
  if (this != &other) {
    // Nothing to assign yet
  }
  return *this;
}

FileHandler::~FileHandler() {}

// -----------------------------------------------------------------------------
// Public Methods
// -----------------------------------------------------------------------------

bool FileHandler::serve_file(const std::string& physical_path,
                             HttpResponse& res) {
  (void)physical_path;
  (void)res;
  // TODO(serjimen): Implement serving logic
  return false;
}

// -----------------------------------------------------------------------------
// Private Methods
// -----------------------------------------------------------------------------

std::string FileHandler::_get_mime_type(const std::string& path) {
  size_t dot_pos = path.find_last_of('.');

  // No extension or dot is the last character
  if (dot_pos == std::string::npos || dot_pos == path.length() - 1) {
    return "application/octet-stream";
  }

  std::string extension = path.substr(dot_pos);

  // Declarative MIME mapping for clean code without nested if-else

  static const MimeMap mime_types[] = {{".html", "text/html"},
                                       {".css", "text/css"},
                                       {".js", "application/javascript"},
                                       {".png", "image/png"},
                                       {".jpg", "image/jpeg"},
                                       {".jpeg", "image/jpeg"},
                                       {".txt", "text/plain"},
                                       {NULL, NULL}};

  for (size_t i = 0; mime_types[i].ext != NULL; ++i) {
    if (extension == mime_types[i].ext) {
      return mime_types[i].type;
    }
  }

  return "application/octet-stream";
}
