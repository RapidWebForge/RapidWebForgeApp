#include "backendgenerator.h"
#include <fmt/core.h>
#include <fstream>
#include <inja/inja.hpp>
#include <iostream>
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
        return false;
    }

    nlohmann::json jsonSchema;
    try {
        file >> jsonSchema;
    } catch (const nlohmann::json::parse_error &e) {
        fmt::print(stderr, "Error parsing JSON: {}\n", e.what());
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
    data["transaction"] = transaction.getName();

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

    try {
        std::string result = env.render_file(templatePath, data);
        writeFile(outputPath, result);
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error generating file for {}: {}\n", transaction.getName(), e.what());
    }
}

void BackendGenerator::generateController(const Transaction &transaction)
{
    std::string templatePath = "utils/inja_templates/controller.inja";
    std::string outputPath = projectPath + "/backend/controllers/" + transaction.getName()
                             + "Controller.js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateModel(const Transaction &transaction)
{
    std::string templatePath = "utils/inja_templates/model.inja";
    std::string outputPath = projectPath + "/backend/models/" + transaction.getName() + ".js";
    generateFile(transaction, templatePath, outputPath);
}

void BackendGenerator::generateRoute(const Transaction &transaction)
{
    std::string templatePath = "utils/inja_templates/route.inja";
    std::string outputPath = projectPath + "/backend/routes/" + transaction.getName() + "Route.js";
    // No necesitamos los campos aquí
    generateFile(transaction, templatePath, outputPath, false);
}

void BackendGenerator::generateIndexFiles()
{
    inja::Environment env;

    // Context for index files
    inja::json context = inja::json::array();
    for (const auto &transaction : transactions) {
        context.push_back(transaction.getName());
    }

    // Generate models/index.js
    std::string modelsIndexContent = env.render("utils/inja_templates/modelsIndex.inja", context);
    writeFile(projectPath + "/backend/models/index.js", modelsIndexContent);

    // Generate routes/index.js
    std::string routesIndexContent = env.render("utils/inja_templates/routesIndex.inja", context);
    writeFile(projectPath + "/backend/routes/index.js", routesIndexContent);
}

// Getter for transactions
std::vector<Transaction> BackendGenerator::getTransactions() const
{
    return transactions;
}
