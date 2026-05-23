// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/FileHandler.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <fstream>
#include <sstream>
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
    return true;  // The error response is already set by the helper
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

bool FileHandler::delete_file(const std::string& physical_path,
                              HttpResponse& res) {
  // Delegate validation logic (checks existence, permissions, and directory
  // protection)
  if (!_validate_delete_access(physical_path, res)) {
    return true;  // The error response is already set by the helper
  }

  // Execute the destructive POSIX call
  if (unlink(physical_path.c_str()) == -1) {
    // Race condition or unexpected hardware error
    res.generate_error_response(500);
    return true;
  }

  // Success: 204 No Content is the standard response for a successful DELETE
  res.set_status(204, "No Content");
  return true;
}

void FileHandler::generate_autoindex(const std::string& dir_path,
                                     const std::string& uri,
                                     HttpResponse& res) {
  std::string html_content = _build_autoindex_html(dir_path, uri);

  if (html_content.empty()) {
    // Si devuelve un string vacío, significa que opendir falló
    res.generate_error_response(403);
    return;
  }

  res.set_body(html_content);
  res.add_header("Content-Type", "text/html");

  std::ostringstream cl;
  cl << html_content.length();
  res.add_header("Content-Length", cl.str());
  res.set_status(200, "OK");
}

// -----------------------------------------------------------------------------
// Private Methods
// -----------------------------------------------------------------------------

std::string FileHandler::_build_autoindex_html(const std::string& dir_path,
                                               const std::string& uri) {
  DIR* dir = opendir(dir_path.c_str());
  if (dir == NULL) {
    return "";
  }

  std::string safe_uri = uri;
  if (!safe_uri.empty() && safe_uri[safe_uri.length() - 1] != '/') {
    safe_uri += "/";
  }

  std::ostringstream html;
  html << "<html><head><title>Index of " << uri
       << "</title></head><body><h1>Index of " << uri << "</h1><ul>\n";

  struct dirent* entry;
  while ((entry = readdir(dir)) != NULL) {
    std::string name = entry->d_name;
    if (name == "." || name == "..") {
      continue;
    }
    html << "  <li><a href=\"" << safe_uri << name << "\">" << name
         << "</a></li>\n";
  }
  html << "</ul></body></html>";

  closedir(dir);
  return html.str();
}

bool FileHandler::_validate_file_access(const std::string& path,
                                        HttpResponse& res) {
  struct stat st;

  // Check if file exists and get info
  if (stat(path.c_str(), &st) != 0) {
    res.generate_error_response(404);
    return false;
  }

  // Ensure it is a regular file (not a directory)
  // NGINX returns 403 Forbidden for directories when autoindex is off
  if (S_ISDIR(st.st_mode)) {
    res.generate_error_response(403);
    return false;
  }

  if (!S_ISREG(st.st_mode)) {
    res.generate_error_response(500);
    return false;
  }

  // Check if we have read permissions
  if (access(path.c_str(), R_OK) != 0) {
    res.generate_error_response(403);
    return false;
  }

  return true;
}

bool FileHandler::_validate_delete_access(const std::string& path,
                                          HttpResponse& res) {
  struct stat st;

  // Check if file exists physically before doing anything destructive
  if (stat(path.c_str(), &st) != 0) {
    res.generate_error_response(404);
    return false;
  }

  // Prevent directory deletion (recursive deletion not supported)
  if (S_ISDIR(st.st_mode)) {
    res.generate_error_response(403);
    return false;
  }

  // Ensure the server process has permission to write (and thus delete)
  if (access(path.c_str(), W_OK) != 0) {
    res.generate_error_response(403);
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
