#include "frontendassistant.h"
#include "src/components/frontend-assistant/ui_frontendassistant.h"

FrontendAssistant::FrontendAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendAssistant)
{
    ui->setupUi(this);
    applyStylesFront();
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

void FrontendAssistant::applyStylesFront()
{
    // Estilo para el GroupBox que contiene el formulario (formGroupBox)
    QString formGroupBoxStyle = "border: none; padding: 20px;";

    // Estilo para los campos de entrada (QLineEdit)
    QString lineEditStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                            "background-color: #ffffff; padding: 5px 10px; ";

    // Estilo general para etiquetas (QLabel)
    QString generalLabelStyle = "color: #333333; font-size: 14px; font-weight: normal;";

    // Estilo específico para la etiqueta de información del proyecto
    ui->subTitleLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                     "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");

    ui->formGroupBox->setStyleSheet(formGroupBoxStyle); // GroupBox para el formulario
    ui->portLabel->setStyleSheet(generalLabelStyle);    // Etiqueta "Port"
    ui->portLineEdit->setStyleSheet(lineEditStyle);     // Campo de texto para el puerto
}
