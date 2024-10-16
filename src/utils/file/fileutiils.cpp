#include "fileutiils.h"

#include <fmt/core.h>
#include <fstream>

namespace FileUtils {

void writeFile(const std::string &filePath, const std::string &content)
{
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);

    if (!file) {
        fmt::print(stderr, "Unable to open file for writing: {}\n", filePath);
        return;
    }

    file << content;

    if (!file.good()) {
        fmt::print(stderr, "Error while writing to the file: {}\n", filePath);
    }

    file.close();

    if (file.fail()) {
        fmt::print(stderr, "Error while closing the file: {}\n", filePath);
    }
}

} // namespace FileUtils
