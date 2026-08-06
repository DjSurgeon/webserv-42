// Copyright 2026 serjimen vja-nie dlesieur
#include "handlers/FileHandler.hpp"
#include "config/LocationConfig.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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
                             HttpResponse* res, const Context* ctx,
                             const HttpRequest* req) {
  // Delegate validation logic
  if (!_validate_file_access(physical_path, res, ctx, req)) {
    return true;  // The error response is already set by the helper
  }

  // Open file in binary mode to prevent corruption of images/assets
  std::ifstream file(physical_path.c_str(), std::ios::binary);

  // Read entire file content into a string efficiently
  std::ostringstream ss;
  ss << file.rdbuf();
  std::string file_content = ss.str();

  // Construct the 200 OK response
  if (res) {
    res->set_body(file_content);
    res->add_header("Content-Type", _get_mime_type(physical_path));

    std::ostringstream cl;
    cl << file_content.length();
    res->add_header("Content-Length", cl.str());

    res->set_status(200, "OK");
  }
  return true;
}

std::string FileHandler::_getAvailableFilename(const std::string& directory,
                                              const std::string& filename) {
  std::string base = filename;
  std::string extension;

  std::size_t dot = filename.find_last_of('.');
  if (dot != std::string::npos && dot != 0) {
    base = filename.substr(0, dot);
    extension = filename.substr(dot);
  }

  std::string candidate = directory + "/" + filename;
  int i = 1;
  while (access(candidate.c_str(), F_OK) == 0) {
    std::ostringstream oss;
    oss << directory << "/" << base << "(" << i++ << ")" << extension;
    candidate = oss.str();
  }
  return candidate;
}

bool FileHandler::_uploadRaw(HttpResponse* res, const LocationConfig* loc,
                            const HttpRequest* req) {
  std::string uri = req->get_uri();
  std::size_t pos = uri.find_last_of('/');
  if (pos == std::string::npos || pos == uri.size() - 1) {
    res->generate_error_response(400, loc, req);
    return false;
  }

  //Create the file
  std::string filename = uri.substr(pos + 1);
  std::string filepath = _getAvailableFilename(loc->getRoot() + "/" + loc->getUploadPath(), filename);
  std::cout << "Trying to create file on: " << filepath.c_str() << std::endl;
  std::ofstream file(filepath.c_str(), std::ios::binary);
  if (!file.is_open()) {
    res->generate_error_response(500, loc, req);
    return false;
  }
  file.write(req->get_body().data(), req->get_body().size());
  file.close();

  //Construct the 201 response
  if (res) {
    res->set_body("Created");
    //res->add_header("Content-Type", _get_mime_type(physical_path));
    res->add_header("Content-Length", "7");
    res->set_status(201, "Created");
  }
  return true;
}

bool FileHandler::_uploadMultipart(HttpResponse* res, const LocationConfig* loc,
                                  const HttpRequest* req) {
  (void)res;
  (void)loc;
  (void)req;
  return false;
}

bool FileHandler::upload_file(const std::string& physical_path,
                             HttpResponse* res, const Context* ctx,
                             const HttpRequest* req) {
  (void) physical_path;
  const LocationConfig* loc = dynamic_cast<const LocationConfig*>(ctx);
  if (!loc || loc->getUploadPath().empty()) {
    res->generate_error_response(403, ctx, req);
    return false;
  }

  const std::map<std::string, std::string>& headers = req->get_headers();
  std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
  if (it != headers.end() && it->second.find("multipart/form-data") != std::string::npos)
    return _uploadMultipart(res, loc, req);
  else
    return _uploadRaw(res, loc, req);
}

bool FileHandler::delete_file(const std::string& physical_path,
                              HttpResponse* res, const Context* ctx,
                              const HttpRequest* req) {
  // Delegate validation logic (checks existence, permissions, and directory
  // protection)
  if (!_validate_delete_access(physical_path, res, ctx, req)) {
    return true;  // The error response is already set by the helper
  }

  // Execute the destructive POSIX call
  if (unlink(physical_path.c_str()) == -1) {
    // Race condition or unexpected hardware error
    if (res) {
      res->generate_error_response(500, ctx, req);
    }
    return true;
  }

  // Success: 204 No Content is the standard response for a successful DELETE
  if (res) {
    res->set_status(204, "No Content");
  }
  return true;
}



void FileHandler::generate_autoindex(const std::string& dir_path,
                                     const std::string& uri, HttpResponse* res,
                                     const Context* ctx,
                                     const HttpRequest* req) {
  if (!res) {
    return;
  }
  std::string html_content = _build_autoindex_html(dir_path, uri);

  if (html_content.empty()) {
    // Si devuelve un string vacío, significa que opendir falló
    res->generate_error_response(403, ctx, req);
    return;
  }

  res->set_body(html_content);
  res->add_header("Content-Type", "text/html");

  std::ostringstream cl;
  cl << html_content.length();
  res->add_header("Content-Length", cl.str());
  res->set_status(200, "OK");
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
                                        HttpResponse* res, const Context* ctx,
                                        const HttpRequest* req) {
  struct stat st;

  // Check if file exists and get info
  if (stat(path.c_str(), &st) != 0) {
    if (res) {
      res->generate_error_response(404, ctx, req);
    }
    return false;
  }

  // Ensure it is a regular file (not a directory)
  // NGINX returns 403 Forbidden for directories when autoindex is off
  if (S_ISDIR(st.st_mode)) {
    if (res) {
      res->generate_error_response(403, ctx, req);
    }
    return false;
  }

  if (!S_ISREG(st.st_mode)) {
    if (res) {
      res->generate_error_response(500, ctx, req);
    }
    return false;
  }

  // Check if we have read permissions
  if (access(path.c_str(), R_OK) != 0) {
    if (res) {
      res->generate_error_response(403, ctx, req);
    }
    return false;
  }

  return true;
}

bool FileHandler::_validate_delete_access(const std::string& path,
                                          HttpResponse* res, const Context* ctx,
                                          const HttpRequest* req) {
  struct stat st;

  // Check if file exists physically before doing anything destructive
  if (stat(path.c_str(), &st) != 0) {
    if (res) {
      res->generate_error_response(404, ctx, req);
    }
    return false;
  }

  // Prevent directory deletion (recursive deletion not supported)
  if (S_ISDIR(st.st_mode)) {
    if (res) {
      res->generate_error_response(403, ctx, req);
    }
    return false;
  }

  // Ensure the server process has permission to write (and thus delete)
  if (access(path.c_str(), W_OK) != 0) {
    if (res) {
      res->generate_error_response(403, ctx, req);
    }
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
                                       {".svg", "image/svg+xml"},
                                       {".txt", "text/plain"},
                                       {NULL, NULL}};

  for (size_t i = 0; mime_types[i].ext != NULL; ++i) {
    if (extension == mime_types[i].ext) {
      return mime_types[i].type;
    }
  }

  return "application/octet-stream";
}
