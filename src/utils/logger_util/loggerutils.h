#ifndef LOGGERUTILS_H
#define LOGGERUTILS_H

#include <QString>
#include "../../core/logging/actionloggerjson.h"
#include <string>

namespace LoggerUtils {
void logPropertyChange(ActionLoggerJson &logger,
                       const QString &property,
                       const QString &newValue,
                       const std::string &componentType);
};

#endif // LOGGERUTILS_H
