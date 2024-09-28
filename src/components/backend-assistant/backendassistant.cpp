#include "backendassistant.h"
#include "ui_backendassistant.h"
#include <QMessageBox>

BackendAssistant::BackendAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackendAssistant),
    summaryAssistant(new SummaryAssistant(this))  // Inicializa la pantalla de resumen
{
    ui->setupUi(this);
    // Conectar el botón Next al slot correspondiente
    connect(ui->nextButton, &QPushButton::clicked, this, &BackendAssistant::on_nextButton_clicked);
}

BackendAssistant::~BackendAssistant()
{
    delete ui;
    delete summaryAssistant;  // Eliminar la pantalla de resumen cuando se destruya BackendAssistant

}

void BackendAssistant::on_nextButton_clicked()
{

    QString port = ui->portLineEdit->text();

    // Verificar que el campo de puerto no esté vacío antes de continuar
    if (port.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in the port field.");
        return;
    }

    // Pasar los datos a la pantalla de resumen
    //summaryAssistant->setProjectSummary("projectName", "frontendFramework", "backendFramework", "databaseFramework");

    // Cerrar la pantalla de backend en lugar de ocultarla
    this->close();
    summaryAssistant->show();
}
