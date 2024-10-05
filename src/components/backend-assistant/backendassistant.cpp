#include "backendassistant.h"
#include "src/components/backend-assistant/ui_backendassistant.h"

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

std::string BackendAssistant::isValid(Project &project)
{
    std::string port = ui->portLineEdit->text().toStdString();

    if (port.empty()) {
        return "Please fill in the port field.";
    } else if (port.length() != 4) {
        return "Port must be exactly 4 digits.";
    } else if (!std::all_of(port.begin(), port.end(), ::isdigit)) {
        return "Port must contain only numeric digits.";
    } else {
        project.setBackendPort(port);
    }
    return "";
}
