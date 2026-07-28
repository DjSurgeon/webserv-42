#include "utils/FileUtils.hpp"

bool FileUtils::isValidExtension(const std::string& filename,
                                 const std::string& extension) {
  if (filename.length() < extension.length()) {
    return false;
  }
  return filename.compare(filename.length() - extension.length(),
                          extension.length(), extension) == 0;
}
