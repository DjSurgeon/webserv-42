// Copyright 2026 raperez- serjimen
#ifndef INCLUDE_UTILS_FILEUTILS_HPP_
#define INCLUDE_UTILS_FILEUTILS_HPP_

#include <string>

class FileUtils {
 public:
  static bool isValidExtension(const std::string& filename,
                               const std::string& extension);

 private:
  FileUtils();  // Prevent instantiation
  FileUtils(const FileUtils&);
  FileUtils& operator=(const FileUtils&);
  ~FileUtils();
};

#endif  // INCLUDE_UTILS_FILEUTILS_HPP_
