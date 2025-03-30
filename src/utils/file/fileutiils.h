#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>

namespace FileUtils {
std::string readFile(const std::string &filePath);
void writeFile(const std::string &filePath, const std::string &content);
}

#endif // FILE_UTILS_H
