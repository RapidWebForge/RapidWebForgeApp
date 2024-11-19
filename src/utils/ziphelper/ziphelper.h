#ifndef ZIPHELPER_H
#define ZIPHELPER_H
#include <QFile>
#include <string>

class ZipHelper
{
public:
    ZipHelper();
    static bool unzip(const std::string &zipPath, const std::string &extractPath);
    static bool unzip(const QByteArray &zipData, const std::string &extractPath);
};

#endif // ZIPHELPER_H
