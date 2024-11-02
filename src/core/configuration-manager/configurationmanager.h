#ifndef CONFIGURATIONMANAGER_H
#define CONFIGURATIONMANAGER_H

#include "../../models/configuration/configuration.h"
#include <functional>
#include <optional>
#include <sqlite3.h>
#include <string>
#include <vector>

class ConfigurationManager
{
private:
    void createConfigDatabase();
    Configuration configuration;

public:
    ConfigurationManager();
    bool executeSQL(sqlite3 *db,
                    const std::string &sql,
                    const std::function<void(sqlite3_stmt *)> &bindParams,
                    const std::function<void(sqlite3_stmt *)> &processResults = nullptr) const;
    Configuration getConfiguration() const;
    void setConfiguration(const Configuration &configuration);
};

#endif // CONFIGURATIONMANAGER_H
