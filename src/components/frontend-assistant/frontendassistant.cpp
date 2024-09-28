#include "frontendassistant.h"
#include "ui_frontendassistant.h"
#include <QMessageBox>

FrontendAssistant::FrontendAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrontendAssistant),
    backendAssistant(new BackendAssistant(this))  // Inicializa la cuarta pantalla
{
    ui->setupUi(this);
    // Conectar el botón Next al slot correspondiente
    connect(ui->nextButton, &QPushButton::clicked, this, &FrontendAssistant::on_nextButton_clicked);
}

FrontendAssistant::~FrontendAssistant()
{
    delete ui;
    delete backendAssistant;  // Eliminar la pantalla de backend cuando se destruya FrontendAssistant
}

void FrontendAssistant::on_nextButton_clicked()
{
    QString port = ui->portLineEdit->text();

    // Verificar que el campo de puerto no esté vacío antes de continuar
    if (port.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in the port field.");
        return;
    }

    // Ocultar la pantalla actual y mostrar la pantalla de Backend
    this->hide();
    backendAssistant->show();
}
