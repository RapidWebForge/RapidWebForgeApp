#include "summaryassistant.h"
#include "src/components/summary-assistant/ui_summaryassistant.h"

SummaryAssistant::SummaryAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SummaryAssistant)
{
    ui->setupUi(this);
    applyStylesSummary();
}

SummaryAssistant::~SummaryAssistant()
{
    delete ui;
}

// TODO: use a class to get information
/*
void SummaryAssistant::setProjectSummary(const QString &name,
                                         const QString &frontend,
                                         const QString &backend,
                                         const QString &database)
{
    ui->projectNameLineEdit->setText(name);

    ui->frontendLabel->setText(frontend);
    ui->backendLabel->setText(backend);
    ui->databaseLabel->setText(database);
}
*/

void SummaryAssistant::applyStylesSummary()
{
    // Estilo general para etiquetas (QLabel)
    QString generalLabelStyle = "color: #333333; font-size: 14px; font-weight: normal;";

    // Estilo para los campos de entrada (QLineEdit)
    QString lineEditStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                            "background-color: #ffffff; padding: 5px 10px; ";

    // Aplicar los estilos a cada elemento de la interfaz
    // Estilo específico para la etiqueta de información del proyecto
    ui->subTitleLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                     "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");

    ui->projectLabel->setStyleSheet(generalLabelStyle);
    ui->projectNameLineEdit->setStyleSheet(
        lineEditStyle); // Campo de texto para el nombre del proyecto
}
