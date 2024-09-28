#include "backendassistant.h"
#include "ui_backendassistant.h"
#include <QMessageBox>

BackendAssistant::BackendAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackendAssistant)
{
    ui->setupUi(this);
}

BackendAssistant::~BackendAssistant()
{
    delete ui;

}

void BackendAssistant::on_nextButton_clicked()
{

    QString port = ui->portLineEdit->text();

    // Verificar que el campo de puerto no esté vacío antes de continuar
    if (port.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in the port field.");
        return;
    }
}
