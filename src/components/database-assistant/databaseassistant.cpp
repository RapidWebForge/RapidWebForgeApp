#include "databaseassistant.h"
#include "src/components/database-assistant/ui_databaseassistant.h"

DatabaseAssistant::DatabaseAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DatabaseAssistant)
{
    ui->setupUi(this);
    applyStylesDatabase();
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
void DatabaseAssistant::applyStylesDatabase()
{
    // Estilo general para etiquetas (QLabel)
    QString generalLabelStyle = "color: #333333; font-size: 14px; font-weight: normal;";

    // Estilo específico para la etiqueta de información del proyecto
    ui->subTitleLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                     "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");

    // Estilo para las etiquetas de los campos (Server, Port, User, Password, Database)
    ui->serverLabel->setStyleSheet(generalLabelStyle);
    ui->portLabel->setStyleSheet(generalLabelStyle);
    ui->userLabel->setStyleSheet(generalLabelStyle);
    ui->passwordLabel->setStyleSheet(generalLabelStyle);
    ui->databaseLabel->setStyleSheet(generalLabelStyle);

    // Estilo para los campos de entrada (QLineEdit)
    QString lineEditStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                            "background-color: #ffffff; padding: 5px 10px; ";
    ui->serverLineEdit->setStyleSheet(lineEditStyle);
    ui->portLineEdit->setStyleSheet(lineEditStyle);
    ui->userLineEdit->setStyleSheet(lineEditStyle);
    ui->passwordLineEdit->setStyleSheet(lineEditStyle);

    // Estilo para la entrada de contraseña (Password)
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    // Estilo para el combo box de la base de datos (QComboBox)
    ui->databaseComboBox->setStyleSheet(
        "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
        "background-color: #ffffff; padding: 5px 10px; ");

    // Estilo para el group box que contiene los elementos (QGroupBox)
    ui->formGroupBox->setStyleSheet(" border-radius: 10px;  ");

    // Estilo para el botón de "Test Connection" y otros botones
    QString buttonStyle = "background-color: #007bff; color: #ffffff; font-size: 14px; "
                          "border: none; border-radius: 5px; padding: 8px 16px; font-weight: bold;";
    QString buttonHoverStyle = "background-color: #0056b3;";

    ui->testConnectionButton->setStyleSheet(buttonStyle + "QPushButton:hover { " + buttonHoverStyle
                                            + " }");

    // Ajustes del layout para que los elementos se alineen mejor
    ui->gridLayout->setHorizontalSpacing(5);            // Espaciado horizontal entre columnas
    ui->gridLayout->setVerticalSpacing(10);             // Espaciado vertical entre filas
    ui->gridLayout->setContentsMargins(110, 0, 110, 0); // Márgenes del layout dentro del group box

    QString testConnectionButtonStyle = "QPushButton {"
                                        "    background-color: #28a745;"
                                        "    color: #ffffff;"
                                        "    font-size: 14px;"
                                        "    border-radius: 5px;"
                                        "    padding: 6px 10px;"
                                        "    font-weight: normal;"
                                        "    text-align: center;"
                                        "}"
                                        "QPushButton:hover {"
                                        "    background-color: #218838;"
                                        "}"
                                        "QPushButton:pressed {"
                                        "    background-color: #1e7e34;"
                                        "}"
                                        "QPushButton:disabled {"
                                        "    background-color: #c3e6cb;"
                                        "    color: #ffffff;"
                                        "    border: 1px solid #c3e6cb;"
                                        "}";

    // Aplicar estilos al botón
    ui->testConnectionButton->setStyleSheet(testConnectionButtonStyle);

    // Alineación y tamaño del icono (si existe un QLabel para el icono)
    if (ui->iconLabel) {
        ui->iconLabel->setAlignment(Qt::AlignCenter);
    }
}
