#include "backendgenerator.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "../../utils/render_callback/rendercallback.h"
#include <boost/algorithm/string.hpp>
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

            /// Si es una Foreign Key, leer la tabla relacionada y almacenar tanto el nombre original como la versión en minúsculas
            if (fieldJson["isForeignKey"].get<bool>()) {
                // Obtener la tabla de la clave foránea
                std::string foreignKeyTable = fieldJson["foreignKeyTable"].get<std::string>();
                field.setForeignKeyTable(foreignKeyTable); // Guardar el nombre original

                // Convertir a minúsculas y guardar
                std::string foreignKeyTableLower = foreignKeyTable;
                std::transform(foreignKeyTableLower.begin(),
                               foreignKeyTableLower.end(),
                               foreignKeyTableLower.begin(),
                               ::tolower);
                field.setForeignKeyTableLower(
                    foreignKeyTableLower); // Asignar la versión en minúsculas
            }

            fields.push_back(field);
        }
        transaction.setFields(fields);

        this->transactions.push_back(transaction);
    }

    this->oldTransactions = this->transactions;
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
                // Asegúrate de que se está añadiendo el campo en minúsculas
                std::string foreignKeyTableLower = field.getForeignKeyTableLower();
                fieldJson["foreignKeyTableLower"]
                    = foreignKeyTableLower; // Guardar la versión en minúsculas
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
    std::vector<TransactionOperation> operations = diffVecs();

    if (operations.empty()) {
        qDebug() << "No changes detected, skipping backend generation.";
        return true;
    }

    // Aplicar cada operación de forma incremental
    // for (auto op : operations) {
    //     switch (op.type) {
    //     case OperationType::Insert:
    //         applyInsertion(op.transaction);
    //         break;
    //     case OperationType::Modify:
    //         applyModification(op.transaction);
    //         break;
    //     case OperationType::Delete:
    //         applyDeletion(op.transaction);
    //         break;
    //     }
    // }

    if (!generateBackendCode()) {
        return false;
    }

    // Generar modelos y servicios para el frontend
    // if (!generateFrontendModels() || !generateFrontendServices()) {
    //     fmt::print(stderr, "Error generating frontend code.\n");
    //     return false;
    // }

    // if (!updateSchema()) {
    //     qDebug() << "Error on Updating Schema";
    //     return false;
    // }

    this->oldTransactions = this->transactions;
    return true;
}

void BackendGenerator::applyInsertion(Transaction &transaction) {}

void BackendGenerator::applyModification(Transaction &transaction) {}

void deleteFile(const QString &path)
{
    QFile file(path);
    if (file.exists()) {
        file.remove();
    }
}

void BackendGenerator::applyDeletion(Transaction &transaction)
{
    // Para una transaction eliminada solo hay que borrar los archivos e imports
    std::string transactionName = transaction.getName();
    std::string transactionLowerName = boost::to_lower_copy(transactionName);
    QString fullPath;
    QString backendPath = QDir(QString::fromStdString(projectPath))
                              .filePath(QString::fromStdString("backend"));
    QString frontendPath = QDir(QString::fromStdString(projectPath))
                               .filePath(QString::fromStdString("frontend"));
    QString frontendSrc = QDir(frontendPath).filePath("src");

    // Backend
    // controllers
    QString controllerFile = QString::fromStdString(transactionLowerName + "Controller.js");
    QString controllerDir = "controllers";
    fullPath = QDir(QDir(backendPath).filePath(controllerDir)).filePath(controllerFile);
    deleteFile(fullPath);
    // models
    QString modelFile = QString::fromStdString(transactionLowerName + ".js");
    QString modelDir = "models";
    fullPath = QDir(QDir(backendPath).filePath(modelDir)).filePath(modelFile);
    deleteFile(fullPath);
    // TODO: Queda pendiente el remover el import del index.js
    // routes
    QString routeFile = QString::fromStdString(transactionLowerName + "Routes.js");
    QString routeDir = "routes";
    fullPath = QDir(QDir(backendPath).filePath(routeDir)).filePath(routeFile);
    deleteFile(fullPath);
    // TODO: Queda pendiente el remover el import del index.js
    // Frontend
    // models
    QString modelFrontFile = QString::fromStdString(transactionName + ".ts");
    fullPath = QDir(QDir(frontendSrc).filePath(modelDir)).filePath(modelFrontFile);
    deleteFile(fullPath);
    // services
    QString serviceFile = QString::fromStdString(transactionName + "Service.ts");
    QString serviceDir = "services";
    fullPath = QDir(QDir(frontendSrc).filePath(serviceDir)).filePath(serviceFile);
    deleteFile(fullPath);
}

void BackendGenerator::generateFileAll(const Transaction &transaction,
                                       const std::string &templatePath,
                                       const std::string &outputPath,
                                       bool includeFields,
                                       const nlohmann::json &allTransactions)
{
    inja::Environment env;
    nlohmann::json data;
    data["name"] = transaction.getName();
    data["nameConst"] = transaction.getNameConst();

    // Verificar si hay campos y si se deben incluir en el contexto
    if (includeFields) {
        data["fields"] = nlohmann::json::array();
        for (const auto &field : transaction.getFields()) {
            nlohmann::json fieldJson;
            fieldJson["name"] = field.getName();
            fieldJson["type"] = field.getType();
            fieldJson["isNull"] = field.getIsNull();
            fieldJson["isUnique"] = field.getIsUnique();
            fieldJson["isForeignKey"] = field.isForeignKey(); // Campo existente

            // Si es una Foreign Key, agregar la tabla relacionada
            if (field.isForeignKey()) {
                fieldJson["foreignKeyTable"] = field.getForeignKeyTable();
                fieldJson["foreignKeyTableLower"]
                    = field.getForeignKeyTableLower(); // Nombre en minúsculas
            }

            data["fields"].push_back(fieldJson);
        }
    }

    // Verificar que `allTransactions` no esté vacío
    if (allTransactions.is_null() || allTransactions.empty()) {
        fmt::print(stderr, "allTransactions is empty or null.\n");
        return;
    }
    data["allTransactions"] = allTransactions;
    // Verificar si el archivo de plantilla existe y puede abrirse
    QFile file(QString::fromStdString(templatePath));
    if (!file.exists()) {
        fmt::print(stderr, "Template file does not exist: {}\n", templatePath);
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "Unable to open template file from resource: {}\n", templatePath);
        return;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();

    // Convertir el contenido a std::string para usarlo con Inja
    std::string templateString = templateContent.toStdString();

    // Depuración: Imprimir el JSON para verificar su estructura antes de renderizar
    fmt::print("Data being passed to Inja:\n{}\n", data.dump(2));

    try {
        // Renderizar la plantilla con Inja
        std::string result = env.render(templateString, data);

        // Escribir el archivo de salida
        writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr,
                   "General error generating file for {}: {}\n",
                   transaction.getName(),
                   e.what());
    }
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
                fieldJson["foreignKeyTable"] = field.getForeignKeyTable(); // Nombre original
                fieldJson["foreignKeyTableLower"]
                    = field.getForeignKeyTableLower(); // Nombre en minúsculas
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

    // Crear un JSON con todas las transacciones
    nlohmann::json allTransactionsData = nlohmann::json::array();
    for (const auto &otherTransaction : transactions) {
        nlohmann::json transactionJson;
        transactionJson["name"] = otherTransaction.getName();
        transactionJson["nameConst"] = otherTransaction.getNameConst();
        allTransactionsData.push_back(transactionJson);
    }
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
    generateFile(transaction, templatePath, outputPath);
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

    try {
        env.add_callback("render_type", 1, [&env](inja::Arguments &args) -> std::string {
            return RenderCallback::renderTypeFrontendModel(env, args);
        });
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error adding callback: {}\n", e.what());
    }

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

std::vector<Transaction> *BackendGenerator::getTransactions()
{
    return &transactions;
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

std::vector<TransactionOperation> BackendGenerator::diffVecs()
{
    std::vector<TransactionOperation> ops;

    for (const Transaction &newTx : transactions) {
        auto it = std::find_if(oldTransactions.begin(),
                               oldTransactions.end(),
                               [&](const Transaction &existingTx) {
                                   return existingTx.getName() == newTx.getName();
                               });

        if (it == oldTransactions.end()) {
            ops.emplace_back(OperationType::Insert, newTx);
        } else if (newTx.isDifferentFrom(*it)) {
            ops.emplace_back(OperationType::Modify, newTx);
        }
    }

    for (const Transaction &oldTx : oldTransactions) {
        auto it = std::find_if(transactions.begin(),
                               transactions.end(),
                               [&](const Transaction &newTx) {
                                   return newTx.getName() == oldTx.getName();
                               });

        if (it == transactions.end()) {
            ops.emplace_back(OperationType::Delete, oldTx);
        }
    }

    return ops;
}

bool BackendGenerator::isProgressSaved()
{
    std::vector<TransactionOperation> operations = diffVecs();

    if (operations.empty()) {
        qDebug() << "No changes detected.";
        return true;
    } else {
        qDebug() << "Changes detected.";
        return false;
    }
}
