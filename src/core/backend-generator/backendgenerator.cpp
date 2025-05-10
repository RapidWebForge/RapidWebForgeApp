#include "backendgenerator.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../utils/file/fileutiils.h"
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

// Cargar el backend.json
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
    this->transactions.clear();

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

// Generar backend a partir de un JSON (usado principalmente para generar el código inicial en un template)
bool BackendGenerator::generateInitialBackendCode()
{
    if (!loadSchema()) {
        fmt::print(stderr, "generateInitialBackendCode: Failed to load schema.\n");
        return false;
    }

    // Generar los archivos de backend
    for (auto &transaction : transactions) {
        if (!applyInsertion(transaction)) {
            fmt::print(stderr, "❌ Failed generating transaction\n");
            return false;
        }
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
    for (auto op : operations) {
        switch (op.type) {
        case OperationType::Insert:
            applyInsertion(op.transaction);
            break;
        case OperationType::Modify:
            applyModification(op.transaction);
            break;
        case OperationType::Delete:
            applyDeletion(op.transaction);
            break;
        }
    }

    if (!updateSchema()) {
        qDebug() << "Error on Updating Schema";
        return false;
    }

    this->oldTransactions = this->transactions;
    return true;
}

bool BackendGenerator::runEditorScript(const std::vector<std::string> &stdArgs)
{
    // 1. Volcar recurso interno a un archivo temporal
    const QString resourcePath = ":/babel/editorBackend";
    QFile resourceFile(resourcePath);
    if (!resourceFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to open resource: {}\n", resourcePath.toStdString());
        return false;
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString tempFilePath = tempDir + "/editorBack.js";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        fmt::print(stderr, "❌ Unable to write temporary editorBack.js\n");
        return false;
    }
    tempFile.write(resourceFile.readAll());
    tempFile.close();

    // 2. Preparar QProcess
    ConfigurationManager configurationManager;
    const QString bunPath = QString::fromStdString(
        configurationManager.getConfiguration().getBunPath());
    const QString backendDir = QDir::toNativeSeparators(QString::fromStdString(projectPath)
                                                        + "/backend");

    // 3. Construir lista de argumentos
    QStringList qArgs;
    qArgs << "run" << "edit" << "--" << tempFilePath;
    for (const auto &s : stdArgs) {
        qArgs << QString::fromStdString(s);
    }

    // 4. Configurar y lanzar el proceso, heredando stdout/stderr
    QProcess proc;
    proc.setProgram(bunPath);
    proc.setArguments(qArgs);
    proc.setWorkingDirectory(backendDir);
    proc.setProcessChannelMode(QProcess::ForwardedChannels);

    proc.start();
    if (!proc.waitForFinished(-1)) {
        qWarning() << "El proceso no terminó correctamente.";
        return false;
    } else {
        // qDebug() << "Proceso backend terminado con código:" << proc.exitCode();
        if (proc.exitCode() == 1)
            return false;
        else
            return true;
    }
}

bool BackendGenerator::applyInsertion(Transaction &transaction)
{
    // Generar controlador, modelo y ruta para el backend
    generateController(transaction);
    generateModel(transaction);
    generateRoute(transaction);
    // Generar modelos y servicios para el frontend
    generateFrontendModel(transaction);
    generateFrontendService(transaction);
    // Editor.js
    QString backendPath = QDir(QString::fromStdString(projectPath)).filePath("backend");
    std::vector<std::string> args = {backendPath.toStdString(), "insert", transaction.getName()};

    // Run script
    return runEditorScript(args);
}

bool BackendGenerator::applyModification(Transaction &transaction)
{
    // Transaction a JSON
    nlohmann::json transactionJson;
    transactionJson["name"] = transaction.getName();

    // Crear el arreglo de fields por cada transaction
    transactionJson["fields"] = nlohmann::json::array();
    for (const auto &field : transaction.getFields()) {
        // Crear un objeto JSON por cada field
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

    std::string payloadJson = transactionJson.dump();

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString payloadPath = tempPath + "/payload.json";
    QFile payloadFile(payloadPath);

    if (payloadFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream out(&payloadFile);
        out << QString::fromStdString(payloadJson);
        payloadFile.close();
    } else {
        fmt::print(stderr, "❌ Unable to write temporary payload.json\n");
        return false;
    }

    // Editor.js
    std::vector<std::string> args = {projectPath, "modify", payloadPath.toStdString()};

    // Run script
    return runEditorScript(args);
}

bool BackendGenerator::applyDeletion(Transaction &transaction)
{
    auto toQString = [](const std::string &s) { return QString::fromStdString(s); };
    auto buildPath = [](const QString &base, const QString &subdir, const QString &file) {
        return QDir(QDir(base).filePath(subdir)).filePath(file);
    };

    // Para una transaction eliminada solo hay que borrar los archivos e imports
    QString transactionNameQString = toQString(transaction.getName());
    QString transactionLowerNameQString = toQString(boost::to_lower_copy(transaction.getName()));

    QString backendPath = QDir(toQString(projectPath)).filePath("backend");
    QString frontendPath = QDir(toQString(projectPath)).filePath("frontend");
    QString frontendSrc = QDir(frontendPath).filePath("src");

    // Backend
    // controllers
    FileUtils::deleteFile(
        buildPath(backendPath, "controllers", transactionLowerNameQString + "Controller.js"));
    // models
    FileUtils::deleteFile(buildPath(backendPath, "models", transactionLowerNameQString + ".js"));
    // routes
    FileUtils::deleteFile(
        buildPath(backendPath, "routes", transactionLowerNameQString + "Routes.js"));

    // Frontend
    // models
    FileUtils::deleteFile(buildPath(frontendSrc, "models", transactionNameQString + ".ts"));
    // services
    FileUtils::deleteFile(buildPath(frontendSrc, "services", transactionNameQString + "Service.ts"));

    // Editor.js
    std::vector<std::string> args = {backendPath.toStdString(), "delete", transaction.getName()};

    // Run script
    return runEditorScript(args);
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

void BackendGenerator::generateFrontendModel(const Transaction &transaction)
{
    inja::Environment env;

    try {
        env.add_callback("render_type", 1, [&env](inja::Arguments &args) -> std::string {
            return RenderCallback::renderTypeFrontendModel(env, args);
        });
        env.add_callback("render_default_type", 1, [&env](inja::Arguments &args) -> std::string {
            return RenderCallback::renderDefaultTypeFrontendModel(env, args);
        });
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error adding callbacks: {}\n", e.what());
    }

    std::string modelTemplatePath = ":/inja/frontend/model";

    // Cargar la plantilla para modelos
    QFile file(QString::fromStdString(modelTemplatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open model template file from resource: {}\n",
                   modelTemplatePath);
        return;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();
    std::string templateString = templateContent.toStdString();

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
    std::string outputPath = projectPath + "/frontend/src/models/" + transaction.getName() + ".ts";
    try {
        std::string result = env.render(templateString, data);
        writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating model for {}: {}\n", transaction.getName(), e.what());
    }
}

void BackendGenerator::generateFrontendService(const Transaction &transaction)
{
    inja::Environment env;
    std::string serviceTemplatePath = ":/inja/frontend/service";

    // Cargar la plantilla para servicios
    QFile file(QString::fromStdString(serviceTemplatePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmt::print(stderr,
                   "Unable to open service template file from resource: {}\n",
                   serviceTemplatePath);
        return;
    }

    QTextStream in(&file);
    QString templateContent = in.readAll();
    file.close();
    std::string templateString = templateContent.toStdString();

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
        fmt::print(stderr, "Error generating service for {}: {}\n", transaction.getName(), e.what());
    }
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

std::vector<TransactionOperation> BackendGenerator::diffVecs()
{
    std::vector<TransactionOperation> ops;

    for (const Transaction &newTx : this->transactions) {
        auto it = std::find_if(this->oldTransactions.begin(),
                               this->oldTransactions.end(),
                               [&](const Transaction &existingTx) {
                                   return existingTx.getName() == newTx.getName();
                               });

        if (it == this->oldTransactions.end()) {
            ops.emplace_back(OperationType::Insert, newTx);
        } else if (newTx.isDifferentFrom(*it)) {
            ops.emplace_back(OperationType::Modify, newTx);
        }
    }

    for (const Transaction &oldTx : this->oldTransactions) {
        auto it = std::find_if(transactions.begin(),
                               transactions.end(),
                               [&](const Transaction &newTx) {
                                   return newTx.getName() == oldTx.getName();
                               });

        if (it == this->transactions.end()) {
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
