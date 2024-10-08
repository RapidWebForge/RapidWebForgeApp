#include "backendgenerator.h"
#include <QFile>
#include <QTextStream>
#include <fmt/core.h>
#include <fstream>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

// Constructor
BackendGenerator::BackendGenerator(const std::string &projectPath)
    : projectPath(projectPath)
{}

// Loading database schema from JSON file
bool BackendGenerator::loadSchema()
{
    std::ifstream file(projectPath + "/backend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/backend.json\n", projectPath);

        // Create backend.json
        nlohmann::json backendJson;
        backendJson["transactions"] = nlohmann::json::array();

        std::ofstream jsonFile(projectPath + "/backend.json");
        if (!jsonFile.is_open()) {
            fmt::print(stderr, "Failed to create backend.json\n");
            return false;
        }

        jsonFile << backendJson.dump(2);
        jsonFile.close();

        return false;
    }

    nlohmann::json jsonSchema;
    try {
        file >> jsonSchema;
    } catch (const nlohmann::json::parse_error &e) {
        fmt::print(stderr, "Error parsing JSON: {}\n", e.what());
        return false;
    }

    // Verify if have transactions
    if (jsonSchema.contains("transactions") && jsonSchema["transactions"].is_array()) {
        if (jsonSchema["transactions"].empty()) {
            fmt::print(stderr, "The backend.json file exists but has no transactions.\n");
            return false;
        }
    } else {
        fmt::print(stderr, "The backend.json file exists but has an invalid structure.\n");
        return false;
    }

    parseJson(jsonSchema);

    return true;
}

// Write files
void BackendGenerator::writeFile(const std::string &filePath, const std::string &content)
{
    // Open file in write mode
    std::ofstream file(filePath);

    // Check open file
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open file for writing: {}\n", filePath);
        return;
    }

    // Write content
    file << content;

    file.close();

    // Optional verification
    if (file.fail()) {
        fmt::print(stderr, "Error while writing and closing the file: {}\n", filePath);
    }
}

// Parse the JSON file and convert it to a vector of transactions and fields
void BackendGenerator::parseJson(const nlohmann::json &jsonSchema)
{
    for (const auto &transactionJson : jsonSchema["transactions"]) {
        Transaction transaction;
        transaction.setName(transactionJson["name"].get<std::string>());
        transaction.setNameConst(transactionJson["nameConst"].get<std::string>());

        std::vector<Field> fields;
        for (const auto &fieldJson : transactionJson["fields"]) {
            Field field;
            field.setName(fieldJson["name"].get<std::string>());
            field.setType(fieldJson["type"].get<std::string>());
            field.setIsNull(fieldJson["isNull"].get<bool>());
            field.setIsUnique(fieldJson["isUnique"].get<bool>());

            fields.push_back(field);
        }
        transaction.setFields(fields);

        transactions.push_back(transaction);
    }
}

// Generate the backend code using Inja templates
bool BackendGenerator::generateBackendCode()
{
    for (const auto &transaction : transactions) {
        generateController(transaction);
        generateModel(transaction);
        generateRoute(transaction);
        generateIndexFiles();
    }

    return true;
}

void BackendGenerator::generateFile(const Transaction &transaction,
                                    const std::string &templatePath,
                                    const std::string &outputPath,
                                    bool includeFields)
{
    inja::Environment env;
    nlohmann::json data;
    data["name"] = transaction.getName();
    data["nameConst"] = transaction.getNameConst();

    if (includeFields) {
        data["fields"] = nlohmann::json::array();
        for (const auto &field : transaction.getFields()) {
            nlohmann::json fieldJson;
            fieldJson["name"] = field.getName();
            fieldJson["type"] = field.getType();
            fieldJson["isNull"] = field.getIsNull();
            fieldJson["isUnique"] = field.getIsUnique();

            data["fields"].push_back(fieldJson);
        }
    }

    QFile file(QString::fromStdString(templatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    // Convertir el contenido a std::string para usarlo con Inja
    std::string templateString = templateContent.toStdString();

    try {
        // Renderizar con Inja usando el contenido del archivo como una cadena
        std::string result = env.render(templateString, data);
        writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating file for {}: {}\n", transaction.getName(), e.what());
    }
}

void BackendGenerator::generateController(const Transaction &transaction)
{
    std::string templatePath = ":/inja_templates/controllers";
    std::string outputPath = projectPath + "/backend/controllers/" + transaction.getNameConst()
                             + "Controller.js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateModel(const Transaction &transaction)
{
    std::string templatePath = ":/inja_templates/models";
    std::string outputPath = projectPath + "/backend/models/" + transaction.getNameConst() + ".js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateRoute(const Transaction &transaction)
{
    std::string templatePath = ":/inja_templates/routes";
    std::string outputPath = projectPath + "/backend/routes/" + transaction.getNameConst()
                             + "Routes.js";
    generateFile(transaction, templatePath, outputPath, false);
}

void BackendGenerator::generateIndexFiles()
{
    inja::Environment env;

    // Crear un JSON para almacenar los nombres de las transacciones
    nlohmann::json transactionsNames = nlohmann::json::array();
    for (const auto &transaction : transactions) {
        nlohmann::json name;
        name["name"] = transaction.getName();
        name["nameConst"] = transaction.getNameConst();
        transactionsNames.push_back(name); // Añadir cada nombre de transacción al array
    }

    // Crear el contexto de datos para inja
    nlohmann::json context;
    context["transactions"] = transactionsNames; // Añadir transacciones al contexto

    // Ruta al template de modelsIndex
    QString modelsIndexTemplatePath = ":/inja_templates/modelsIndex";
    QFile modelsIndexFile(modelsIndexTemplatePath);
    if (!modelsIndexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open template file from resource: {}\n",
                   modelsIndexTemplatePath.toStdString());
        return;
    }

    QTextStream modelsIndexStream(&modelsIndexFile);
    QString modelsIndexTemplateContent = modelsIndexStream.readAll();
    modelsIndexFile.close();

    // Añadir credenciales a `context` para usar en el template de rutas
    nlohmann::json credentials = {{"dbname", "your_database_name"},
                                  {"user", "your_database_user"},
                                  {"password", "your_database_password"},
                                  {"host", "localhost"}};
    context["credentials"] = credentials;

    // Renderizar el template de modelsIndex con el contexto
    std::string modelsIndexResult = env.render(modelsIndexTemplateContent.toStdString(), context);
    writeFile(projectPath + "/backend/models/index.js", modelsIndexResult);

    // Ruta al template de routesIndex
    QString routesIndexTemplatePath = ":/inja_templates/routesIndex";
    QFile routesIndexFile(routesIndexTemplatePath);
    if (!routesIndexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open template file from resource: {}\n",
                   routesIndexTemplatePath.toStdString());
        return;
    }

    QTextStream routesIndexStream(&routesIndexFile);
    QString routesIndexTemplateContent = routesIndexStream.readAll();
    routesIndexFile.close();

    // Renderizar el template de routesIndex con el contexto
    std::string routesIndexResult = env.render(routesIndexTemplateContent.toStdString(), context);
    writeFile(projectPath + "/backend/routes/index.js", routesIndexResult);
}

// Getter for transactions
const std::vector<Transaction> &BackendGenerator::getTransactions() const
{
    return transactions;
}

std::vector<Transaction> &BackendGenerator::getTransactions()
{
    return transactions;
}
