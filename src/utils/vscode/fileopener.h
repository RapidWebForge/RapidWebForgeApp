#ifndef FILEOPENER_H
#define FILEOPENER_H
#include <QDebug>

#include <QString>

class FileOpener {
public:
    static bool openInVSCode(const std::string &folderPath, const std::string &filePath = "");
};

#endif // FILEOPENER_H
