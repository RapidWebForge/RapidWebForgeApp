#include "frontendassistant.h"
#include "src/components/frontend-assistant/ui_frontendassistant.h"

FrontendAssistant::FrontendAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendAssistant)
{
    ui->setupUi(this);
}

FrontendAssistant::~FrontendAssistant()
{
    delete ui;
}

std::string FrontendAssistant::isValid(Project &project)
{
    std::string port = ui->portLineEdit->text().toStdString();

    if (port.empty()) {
        return "Please fill in the port field.";
    } else if (port.length() != 4) {
        return "Port must be exactly 4 digits.";
    } else if (!std::all_of(port.begin(), port.end(), ::isdigit)) {
        return "Port must contain only numeric digits.";
    } else {
        project.setFrontendPort(port);
    }
    return "";
}
