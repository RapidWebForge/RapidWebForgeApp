#ifndef FILEOPENER_H
#define FILEOPENER_H
#include <QDebug>

#include <QString>

class FileOpener {
public:
    static bool openInVSCode(const std::string &path);
};

#endif // FILEOPENER_H
