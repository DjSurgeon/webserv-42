#ifndef FILEUTILS_HPP
#define FILEUTILS_HPP

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

#endif  // FILEUTILS_HPP
