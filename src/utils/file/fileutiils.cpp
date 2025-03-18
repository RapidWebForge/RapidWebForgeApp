#include "fileutiils.h"

#include <fmt/core.h>
#include <fstream>
#include <sstream>

namespace FileUtils {

std::string readFile(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::in);
    if (!file) {
        fmt::print(stderr, "Unable to open file for reading: {}\n", filePath);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    if (file.fail()) {
        fmt::print(stderr, "Error while closing the file: {}\n", filePath);
    }

    return buffer.str();
}

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
