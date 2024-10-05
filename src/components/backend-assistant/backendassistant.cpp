#include "backendassistant.h"
#include "src/components/backend-assistant/ui_backendassistant.h"

BackendAssistant::BackendAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackendAssistant)
{
    ui->setupUi(this);
    applyStylesBack();
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
void BackendAssistant::applyStylesBack()
{
    // Estilo para el GroupBox que contiene el formulario (formGroupBox)
    QString formGroupBoxStyle = "border: none; padding: 20px;";

    // Estilo general para etiquetas (QLabel)
    QString generalLabelStyle = "color: #333333; font-size: 14px; font-weight: normal;";

    // Estilo para los campos de entrada (QLineEdit)
    QString lineEditStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                            "background-color: #ffffff; padding: 5px 10px; ";

    // Estilo específico para la etiqueta de información del proyecto
    ui->subTitleLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                     "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");

    ui->formGroupBox->setStyleSheet(formGroupBoxStyle); // GroupBox para el formulario
    ui->portLabel->setStyleSheet(generalLabelStyle);    // Etiqueta "Port"
    ui->portLineEdit->setStyleSheet(lineEditStyle);     // Campo de texto para el puerto
}
