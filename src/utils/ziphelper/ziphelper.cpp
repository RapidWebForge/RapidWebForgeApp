#include "ziphelper.h"
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <zip.h>

ZipHelper::ZipHelper() {}

bool ZipHelper::unzip(const std::string &zipPath, const std::string &extractPath)
{
    int err = 0;
    zip *zipfile = zip_open(zipPath.c_str(), 0, &err);
    if (!zipfile) {
        fmt::print(stderr, "Unable to open zip file: {}\n", zipPath);
        return false;
    }

    zip_int64_t numFiles = zip_get_num_entries(zipfile, 0);
    for (zip_uint64_t i = 0; i < numFiles; i++) {
        const char *filename = zip_get_name(zipfile, i, 0);
        if (!filename) {
            fmt::print(stderr, "Unable to get file name from zip\n");
            zip_close(zipfile);
            return false;
        }

        std::string fullFilePath = extractPath + "/" + filename;
        if (filename[strlen(filename) - 1] == '/') {
            // Si es un directorio, lo creamos
            std::filesystem::create_directories(fullFilePath);
        } else {
            // Si es un archivo, lo extraemos
            zip_file *file = zip_fopen_index(zipfile, i, 0);
            if (!file) {
                fmt::print(stderr, "Unable to open file in zip: {}\n", filename);
                zip_close(zipfile);
                return false;
            }

            std::ofstream outFile(fullFilePath, std::ios::binary);
            if (!outFile.is_open()) {
                fmt::print(stderr, "Unable to create output file: {}\n", fullFilePath);
                zip_fclose(file);
                zip_close(zipfile);
                return false;
            }

            char buffer[8192];
            zip_int64_t bytesRead;
            while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                outFile.write(buffer, bytesRead);
            }

            outFile.close();
            zip_fclose(file);
        }
    }

    zip_close(zipfile);
    return true;
}

bool ZipHelper::unzip(const QByteArray &zipData, const std::string &extractPath)
{
    // Crear un zip_error_t en lugar de int
    zip_error_t error;
    zip_error_init(&error); // Inicializar el objeto de error

    // Crear un zip_source desde el QByteArray
    zip_source_t *zipSource = zip_source_buffer_create(zipData.constData(),
                                                       zipData.size(),
                                                       0,
                                                       &error);
    if (!zipSource) {
        fmt::print(stderr,
                   "Unable to create zip source from memory: {}\n",
                   zip_error_strerror(&error));
        zip_error_fini(&error); // Finalizar el objeto de error
        return false;
    }

    // Abrir el archivo zip desde el buffer
    zip *zipfile = zip_open_from_source(zipSource, 0, &error);
    if (!zipfile) {
        fmt::print(stderr, "Unable to open zip from memory: {}\n", zip_error_strerror(&error));
        zip_source_free(zipSource);
        zip_error_fini(&error); // Finalizar el objeto de error
        return false;
    }

    zip_int64_t numFiles = zip_get_num_entries(zipfile, 0);
    for (zip_uint64_t i = 0; i < numFiles; i++) {
        const char *filename = zip_get_name(zipfile, i, 0);
        if (!filename) {
            fmt::print(stderr, "Unable to get file name from zip\n");
            zip_close(zipfile);
            zip_error_fini(&error); // Finalizar el objeto de error
            return false;
        }

        std::string fullFilePath = extractPath + "/" + filename;
        if (filename[strlen(filename) - 1] == '/') {
            // Si es un directorio, lo creamos
            std::filesystem::create_directories(fullFilePath);
        } else {
            // Si es un archivo, lo extraemos
            zip_file *file = zip_fopen_index(zipfile, i, 0);
            if (!file) {
                fmt::print(stderr, "Unable to open file in zip: {}\n", filename);
                zip_close(zipfile);
                zip_error_fini(&error); // Finalizar el objeto de error
                return false;
            }

            std::ofstream outFile(fullFilePath, std::ios::binary);
            if (!outFile.is_open()) {
                fmt::print(stderr, "Unable to create output file: {}\n", fullFilePath);
                zip_fclose(file);
                zip_close(zipfile);
                zip_error_fini(&error); // Finalizar el objeto de error
                return false;
            }

            char buffer[8192];
            zip_int64_t bytesRead;
            while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                outFile.write(buffer, bytesRead);
            }

            outFile.close();
            zip_fclose(file);
        }
    }

    zip_close(zipfile);
    zip_error_fini(&error); // Finalizar el objeto de error
    return true;
}
