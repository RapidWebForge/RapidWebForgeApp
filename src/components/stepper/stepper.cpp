#include "stepper.h"
#include "ui_stepper.h"

Stepper::Stepper(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Stepper)
    , creationAssistant(new CreationAssistant())
    , databaseAssistant(new DatabaseAssistant())
    , frontendAssistant(new FrontendAssistant())
    , summaryAssistant(new SummaryAssistant())
{
    ui->setupUi(this);

    ui->stepsWidget->addWidget(creationAssistant);
    ui->stepsWidget->addWidget(frontendAssistant);
    ui->stepsWidget->addWidget(databaseAssistant);
    ui->stepsWidget->addWidget(summaryAssistant);

    ui->stepsWidget->setCurrentWidget(creationAssistant);
}

Stepper::~Stepper()
{
    delete ui;
}

void Stepper::on_nextButton_clicked()
{
    int currentIndex = ui->stepsWidget->currentIndex();

    if (currentIndex < ui->stepsWidget->count() - 1) {
        ui->stepsWidget->setCurrentIndex(currentIndex + 1);
    }
}


void Stepper::on_backButton_clicked()
{
    int currentIndex = ui->stepsWidget->currentIndex();

    if (currentIndex > 0) {
        ui->stepsWidget->setCurrentIndex(currentIndex - 1);
    }
}
