#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <QString>
#include <string>

namespace FileUtils {
std::string readFile(const std::string &filePath);
void writeFile(const std::string &filePath, const std::string &content);
void deleteFile(const QString &path);
}

#endif // FILE_UTILS_H
