// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/FileHandler.hpp"

#include <unistd.h>

#include <fstream>
#include <sstream>


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
  // Delegate validation logic
  if (!_validate_file_access(physical_path, res)) {
    return true; // The error response is already set by the helper
  }

  // Open file in binary mode to prevent corruption of images/assets
  std::ifstream file(physical_path.c_str(), std::ios::binary);

  // Read entire file content into a string efficiently
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string file_content = ss.str();

  // Construct the 200 OK response
  res.set_body(file_content);
  res.add_header("Content-Type", _get_mime_type(physical_path));

  std::ostringstream cl;
  cl << file_content.length();
  res.add_header("Content-Length", cl.str());

  res.set_status(200, "OK");
  return true;
}

// -----------------------------------------------------------------------------
// Private Methods
// -----------------------------------------------------------------------------

bool FileHandler::_validate_file_access(const std::string& path,
                                        HttpResponse& res) {
  // Check if file exists
  if (access(path.c_str(), F_OK) != 0) {
    res.generate_error_response(404);
    return false;
  }

  // Check if we have read permissions
  if (access(path.c_str(), R_OK) != 0) {
    res.generate_error_response(403);
    return false;
  }

  // Check if we can open the file at all (e.g. not a directory when it shouldn't be)
  std::ifstream test_file(path.c_str());
  if (!test_file.is_open()) {
    res.generate_error_response(500);
    return false;
  }

  return true;
}

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
