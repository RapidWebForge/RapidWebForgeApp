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

std::string FrontendAssistant::isValid()
{
    QString port = ui->portLineEdit->text();

    // TODO: Check a correct port
    return port.isEmpty() ? "Please fill in the port field." : "";
}
