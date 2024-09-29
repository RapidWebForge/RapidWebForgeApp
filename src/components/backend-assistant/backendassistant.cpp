#include "backendassistant.h"
#include "ui_backendassistant.h"

BackendAssistant::BackendAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackendAssistant)
{
    ui->setupUi(this);
}

BackendAssistant::~BackendAssistant()
{
    delete ui;
}

std::string BackendAssistant::isValid()
{
    QString port = ui->portLineEdit->text();

    // TODO: Check a correct port
    return port.isEmpty() ? "Please fill in the port field." : "";
}
