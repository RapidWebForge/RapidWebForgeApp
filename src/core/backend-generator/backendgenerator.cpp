#include "backendgenerator.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <fmt/core.h>
#include <fstream>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

// Constructor
BackendGenerator::BackendGenerator(const std::string &projectPath, const DatabaseData &databaseData)
    : projectPath(projectPath)
    , databaseData(databaseData)
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
            //field.setIsPrimaryKey(fieldJson["isPrimaryKey"].get<bool>()); // Asegúrate de cargar isPrimaryKey
            field.setIsForeignKey(fieldJson["isForeignKey"].get<bool>());

            // Cargar los nuevos constraints desde el JSON
            if (fieldJson.contains("hasCheck")) {
                field.setHasCheck(fieldJson["hasCheck"].get<bool>());
            } else {
                field.setHasCheck(false); // Si no está en el JSON, asigna false por defecto
            }

            if (fieldJson.contains("hasDefault")) {
                field.setHasDefault(fieldJson["hasDefault"].get<bool>());
            } else {
                field.setHasDefault(false); // Si no está en el JSON, asigna false por defecto
            }

            // Si es una Foreign Key, leer la tabla relacionada
            if (fieldJson["isForeignKey"].get<bool>()) {
                field.setForeignKeyTable(fieldJson["foreignKeyTable"].get<std::string>());
            }

            fields.push_back(field);
        }
        transaction.setFields(fields);

        transactions.push_back(transaction);
    }
}

bool BackendGenerator::updateSchema()
{
    nlohmann::json jsonSchema;

    // Create the transactions array
    jsonSchema["transactions"] = nlohmann::json::array();

    // Loop through each transaction
    for (const auto &transaction : transactions) {
        // Create a JSON object for each transaction
        nlohmann::json transactionJson;
        transactionJson["name"] = transaction.getName();
        transactionJson["nameConst"] = transaction.getNameConst();

        // Create a fields array for each transaction
        transactionJson["fields"] = nlohmann::json::array();
        for (const auto &field : transaction.getFields()) {
            // Create a JSON object for each field
            nlohmann::json fieldJson;
            fieldJson["name"] = field.getName();
            fieldJson["type"] = field.getType();
            fieldJson["isNull"] = field.getIsNull();
            fieldJson["isUnique"] = field.getIsUnique();
            //fieldJson["isPrimaryKey"] = field.isPrimaryKey();
            fieldJson["isForeignKey"] = field.isForeignKey();
            // Incluir los nuevos constraints en el JSON
            fieldJson["hasCheck"] = field.getHasCheck();
            fieldJson["hasDefault"] = field.getHasDefault();

            // Si es una Foreign Key, guarda la tabla relacionada
            if (field.isForeignKey()) {
                fieldJson["foreignKeyTable"] = field.getForeignKeyTable();
            }
            // Add the field JSON object to the fields array
            transactionJson["fields"].push_back(fieldJson);
        }

        // Add the transaction JSON object to the transactions array
        jsonSchema["transactions"].push_back(transactionJson);
    }

    // Construct the full file path using projectPath
    std::string filePath = projectPath + "/backend.json";

    // Write the JSON to a file
    std::ofstream jsonFile(filePath);
    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to open the file for writing: {}\n", filePath);
        return false;
    }

    jsonFile << jsonSchema.dump(2);
    jsonFile.close();

    // Check for writing errors
    if (jsonFile.fail()) {
        fmt::print(stderr, "Error while writing and closing the file: {}\n", filePath);
        return false;
    }

    fmt::print("File saved successfully: {}\n", filePath);

    return true;
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

// Regenerate backend with new information
bool BackendGenerator::updateBackendCode()
{
    // Actualizar el esquema del backend
    if (updateSchema()) {
        // Generar el código del backend
        if (!generateBackendCode()) {
            return false;
        }

        // Generar modelos y servicios para el frontend
        if (!generateFrontendModels() || !generateFrontendServices()) {
            fmt::print(stderr, "Error generating frontend code.\n");
            return false;
        }

        return true;
    }
    return false;
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

            // Incluyendo los nuevos campos
            fieldJson["hasCheck"] = field.getHasCheck();      // Nuevo campo
            fieldJson["hasDefault"] = field.getHasDefault();  // Nuevo campo
            fieldJson["isForeignKey"] = field.isForeignKey(); // Campo existente

            // Si es una Foreign Key, incluir la tabla relacionada
            if (field.isForeignKey()) {
                fieldJson["foreignKeyTable"] = field.getForeignKeyTable();
            }
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
    std::string templatePath = ":/inja/backend/controllers";
    std::string outputPath = projectPath + "/backend/controllers/" + transaction.getNameConst()
                             + "Controller.js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateModel(const Transaction &transaction)
{
    std::string templatePath = ":/inja/backend/models";
    std::string outputPath = projectPath + "/backend/models/" + transaction.getNameConst() + ".js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateRoute(const Transaction &transaction)
{
    std::string templatePath = ":/inja/backend/routes";
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
    QString modelsIndexTemplatePath = ":/inja/backend/modelsIndex";
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
    nlohmann::json credentials = {{"dbname", databaseData.getDatabaseName()},
                                  {"user", databaseData.getUser()},
                                  {"password", databaseData.getPassword()},
                                  {"host", databaseData.getServer()}};
    context["credentials"] = credentials;

    // Renderizar el template de modelsIndex con el contexto
    std::string modelsIndexResult = env.render(modelsIndexTemplateContent.toStdString(), context);
    writeFile(projectPath + "/backend/models/index.js", modelsIndexResult);

    // Ruta al template de routesIndex
    QString routesIndexTemplatePath = ":/inja/backend/routesIndex";
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

bool BackendGenerator::generateFrontendModels()
{
    inja::Environment env;
    std::string modelTemplatePath = ":/inja/frontend/model";

    // Cargar la plantilla para modelos
    QFile file(QString::fromStdString(modelTemplatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open model template file from resource: {}\n",
                   modelTemplatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();
    std::string templateString = templateContent.toStdString();

    // Iterar sobre las transacciones para generar modelos
    for (const auto &transaction : transactions) {
        nlohmann::json data;
        data["model_name"] = transaction.getName();
        data["fields"] = nlohmann::json::array();

        for (const auto &field : transaction.getFields()) {
            nlohmann::json fieldJson;
            fieldJson["name"] = field.getName();
            fieldJson["type"] = field.getType();
            data["fields"].push_back(fieldJson);
        }

        // Crear el archivo de modelo en el frontend
        std::string outputPath = projectPath + "/frontend/src/models/" + transaction.getName()
                                 + ".ts";
        try {
            std::string result = env.render(templateString, data);
            writeFile(outputPath, result);
        } catch (const std::exception &e) {
            fmt::print(stderr,
                       "Error generating model for {}: {}\n",
                       transaction.getName(),
                       e.what());
            return false;
        }
    }
    return true;
}

bool BackendGenerator::generateFrontendServices()
{
    inja::Environment env;
    std::string serviceTemplatePath = ":/inja/frontend/service";

    // Cargar la plantilla para servicios
    QFile file(QString::fromStdString(serviceTemplatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open service template file from resource: {}\n",
                   serviceTemplatePath);
        return false;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();
    std::string templateString = templateContent.toStdString();

    // Iterar sobre las transacciones para generar servicios
    for (const auto &transaction : transactions) {
        nlohmann::json data;
        data["model_name"] = transaction.getName();
        data["model_name_lower"] = transaction.getNameConst(); // Nombre en minúsculas

        // Crear el archivo de servicio en el frontend
        std::string outputPath = projectPath + "/frontend/src/services/" + transaction.getName()
                                 + "Service.ts";
        try {
            std::string result = env.render(templateString, data);
            writeFile(outputPath, result);
        } catch (const std::exception &e) {
            fmt::print(stderr,
                       "Error generating service for {}: {}\n",
                       transaction.getName(),
                       e.what());
            return false;
        }
    }
    return true;
}

// Getter
const std::vector<Transaction> &BackendGenerator::getTransactions() const
{
    return transactions;
}

std::vector<Transaction> &BackendGenerator::getTransactions()
{
    return transactions;
}

// Setter
void BackendGenerator::setTransactions(const std::vector<Transaction> &transactions)
{
    this->transactions = transactions;
}

bool BackendGenerator::updateTransactionName(const std::string &currentName,
                                             const std::string &newName,
                                             const std::string &newNameConst)
{
    // Cargar el JSON
    std::ifstream file(projectPath + "/backend.json");
    if (!file.is_open()) {
        fmt::print(stderr, "Unable to open JSON file: {}/backend.json\n", projectPath);
        return false;
    }

    nlohmann::json jsonSchema;
    try {
        file >> jsonSchema;
    } catch (const nlohmann::json::parse_error &e) {
        fmt::print(stderr, "Error parsing JSON: {}\n", e.what());
        return false;
    }

    file.close();

    // Buscar la transacción por el nombre actual
    bool found = false;
    for (auto &transaction : jsonSchema["transactions"]) {
        if (transaction["name"] == currentName) {
            // Actualizar "name" y "nameConst"
            transaction["name"] = newName;
            transaction["nameConst"] = newNameConst;
            found = true;
            break;
        }
    }

    if (!found) {
        fmt::print(stderr, "Transaction with name {} not found.\n", currentName);
        return false;
    }

    // Guardar el JSON actualizado
    std::ofstream jsonFile(projectPath + "/backend.json");
    if (!jsonFile.is_open()) {
        fmt::print(stderr, "Failed to open backend.json for writing\n");
        return false;
    }

    jsonFile << jsonSchema.dump(
        2); // Guardar con una indentación de 2 espacios para mejor legibilidad
    jsonFile.close();

    fmt::print("Transaction updated successfully: name={}, nameConst={}\n", newName, newNameConst);
    return true;
}
