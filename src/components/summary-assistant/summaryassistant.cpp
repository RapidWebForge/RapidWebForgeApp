#include "summaryassistant.h"
#include "ui_summaryassistant.h"

SummaryAssistant::SummaryAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SummaryAssistant)
{
    ui->setupUi(this);
}

SummaryAssistant::~SummaryAssistant()
{
    delete ui;
}
//void SummaryAssistant::setProjectSummary(const QString &name, const QString &frontend, const QString &backend, const QString &database)
//{
    // Configurar la vista del nombre y frameworks seleccionados
    //ui->projectNameLineEdit->setText(name);

    // Cambiar el contenido de los labels o imágenes según los valores pasados
    //ui->frontendLabel->setText(frontend);
    //ui->backendLabel->setText(backend);
    //ui->databaseLabel->setText(database);
//}
