#include "databaseassistant.h"
#include "src/components/database-assistant/ui_databaseassistant.h"

DatabaseAssistant::DatabaseAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseAssistant)
{
    ui->setupUi(this);
}

DatabaseAssistant::~DatabaseAssistant()
{
    delete ui;
}

std::string DatabaseAssistant::isValid(Project &project)
{
    std::string server = ui->serverLineEdit->text().toStdString();
    std::string port = ui->portLineEdit->text().toStdString();
    std::string user = ui->userLineEdit->text().toStdString();
    std::string password = ui->passwordLineEdit->text().toStdString();
    std::string database = ui->databaseComboBox->currentText().toStdString();

    if (server.empty()) {
        return "Choose a server";
    } else {
        project.getDatabaseData().setServer(server);
    }
    if (port.empty()) {
        return "Choose a port";
    } else {
        project.getDatabaseData().setPort(port);
    }
    if (user.empty()) {
        return "Enter your user";
    } else {
        project.getDatabaseData().setUser(user);
    }
    if (password.empty()) {
        return "Enter your password";
    } else {
        project.getDatabaseData().setPassword(password);
    }
    if (database.empty()) {
        return "Enter your database";
    } else {
        project.getDatabaseData().setDatabase(database);
    }

    return "";
}
