#include "databaseassistant.h"
#include "ui_databaseassistant.h"
#include <QMessageBox>

DatabaseAssistant::DatabaseAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseAssistant),
    frontendAssistant(new FrontendAssistant(this))  // Inicializa la tercera pantalla
{
    ui->setupUi(this);
    // Conectar el botón Next al slot correspondiente
    connect(ui->nextButton, &QPushButton::clicked, this, &DatabaseAssistant::on_nextButton_clicked);
}

DatabaseAssistant::~DatabaseAssistant()
{
    delete ui;
    delete frontendAssistant;  // Eliminar la pantalla de frontend cuando se destruya DatabaseAssistant

}

void DatabaseAssistant::on_nextButton_clicked()
{
    QString server = ui->serverLineEdit->text();
    QString port = ui->portLineEdit->text();
    QString user = ui->userLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString database = ui->databaseComboBox->currentText();

    // Verificar que los campos no estén vacíos antes de continuar
    if (server.isEmpty() || port.isEmpty() || user.isEmpty() || database.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all the required fields.");
        return;
    }

    // Ocultar la pantalla actual y mostrar la pantalla de Frontend
    this->hide();
    frontendAssistant->show();
}
