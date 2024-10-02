#include "summaryassistant.h"
#include "src/components/summary-assistant/ui_summaryassistant.h"

SummaryAssistant::SummaryAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SummaryAssistant)
{
    ui->setupUi(this);
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
