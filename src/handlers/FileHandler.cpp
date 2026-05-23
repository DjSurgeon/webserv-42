// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/FileHandler.hpp"

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
  // TODO: Implement serving logic
  return false;
}

// -----------------------------------------------------------------------------
// Private Methods
// -----------------------------------------------------------------------------

std::string FileHandler::_get_mime_type(const std::string& path) {
  (void)path;
  // TODO: Implement MIME type resolution
  return "";
}
