#ifndef BACKENDGENERATOR_H
#define BACKENDGENERATOR_H

#include <QString>
#include "../../models/transaction/transaction.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class BackendGenerator
{
public:
    BackendGenerator(const std::string &projectPath);
    bool loadSchema();
    bool generateBackendCode();
    bool updateSchema();
    bool updateBackendCode();
    void setTransactions(const std::vector<Transaction> &newTransactions);
    const std::vector<Transaction> &getTransactions() const;
    std::vector<Transaction> &getTransactions();

private:
    void generateFile(const Transaction &transaction,
                      const std::string &templatePath,
                      const std::string &outputPath,
                      bool includeColumns = true);
    void generateController(const Transaction &transaction);
    void generateModel(const Transaction &transaction);
    void generateRoute(const Transaction &transaction);
    void generateIndexFiles();
    void writeFile(const std::string &filePath, const std::string &content);
    void parseJson(const nlohmann::json &jsonSchema);

    std::vector<Transaction> transactions;
    std::string projectPath;
};

#endif // BACKENDGENERATOR_H
