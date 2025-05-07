#ifndef BACKENDGENERATOR_H
#define BACKENDGENERATOR_H

#include <QString>
#include "../../models/database-data/databasedata.h"
#include "../../models/transaction-operation/transactionoperation.h"
#include "../../models/transaction/transaction.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class BackendGenerator
{
public:
    BackendGenerator(const std::string &projectPath, const DatabaseData &databaseData);
    // Schema
    bool loadSchema();
    bool updateSchema();
    // Code
    bool generateCode();
    bool updateBackendCode();
    // Getters
    std::vector<Transaction> *getTransactions();
    // Setters
    void setTransactions(const std::vector<Transaction> &transactions);

    // Prevent changes
    bool isProgressSaved();

private:
    std::vector<Transaction> transactions;
    std::vector<Transaction> oldTransactions;
    std::string projectPath;
    DatabaseData databaseData;

    void generateFile(const Transaction &transaction,
                      const std::string &templatePath,
                      const std::string &outputPath,
                      bool includeColumns = true);
    void generateController(const Transaction &transaction);
    void generateModel(const Transaction &transaction);
    void generateRoute(const Transaction &transaction);
    void generateIndexFiles();
    void generateFrontendModel(const Transaction &transaction);
    void generateFrontendService(const Transaction &transaction);
    void writeFile(const std::string &filePath, const std::string &content);
    void parseJson(const nlohmann::json &jsonSchema);
    // Funciones Auxiliares para modificaciones
    std::vector<TransactionOperation> diffVecs();
    void runEditorScript(const std::vector<std::string> &stdArgs);
    void applyInsertion(Transaction &transaction);
    void applyModification(Transaction &transaction);
    void applyDeletion(Transaction &transaction);
};

#endif // BACKENDGENERATOR_H
