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

std::string DatabaseAssistant::isValid()
{
    QString server = ui->serverLineEdit->text();
    QString port = ui->portLineEdit->text();
    QString user = ui->userLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString database = ui->databaseComboBox->currentText();

    if (server.isEmpty()) {
        return "Choose a server";
    } else if (port.isEmpty()) {
        return "Choose a port";
    } else if (user.isEmpty()) {
        return "Enter your user";
    } else if (password.isEmpty()) {
        return "Enter your password";
    } else if (database.isEmpty()) {
        return "Enter your database";
    }

    return "";
}
