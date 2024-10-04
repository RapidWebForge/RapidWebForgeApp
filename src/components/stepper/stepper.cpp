#include "stepper.h"
#include <QMessageBox>
#include "ui_stepper.h"

Stepper::Stepper(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Stepper)
    , creationAssistant(new CreationAssistant())
    , databaseAssistant(new DatabaseAssistant())
    , frontendAssistant(new FrontendAssistant())
    , backendAssistant(new BackendAssistant())
    , summaryAssistant(new SummaryAssistant())
    , newProject()
{
    ui->setupUi(this);

    ui->stepsWidget->addWidget(creationAssistant);
    ui->stepsWidget->addWidget(databaseAssistant);
    ui->stepsWidget->addWidget(frontendAssistant);
    ui->stepsWidget->addWidget(backendAssistant);
    ui->stepsWidget->addWidget(summaryAssistant);

    ui->stepsWidget->setCurrentWidget(creationAssistant);
}

Stepper::~Stepper()
{
    delete ui;
    // delete newProject;
}

void Stepper::on_nextButton_clicked()
{
    // Get the current widget and index
    QWidget *currentWidget = ui->stepsWidget->currentWidget();
    int currentIndex = ui->stepsWidget->currentIndex();

    std::string message = "";

    // Check if exist isValid() method
    if (currentIndex == 1) {
        CreationAssistant *creationAssistantWidget = qobject_cast<CreationAssistant *>(
            currentWidget);
        message = creationAssistantWidget ? creationAssistantWidget->isValid() : "";
    } else if (currentIndex == 2) {
        DatabaseAssistant *databaseAsistantWidget = qobject_cast<DatabaseAssistant *>(currentWidget);
        message = databaseAsistantWidget ? databaseAsistantWidget->isValid() : "";
    } else if (currentIndex == 3) {
        FrontendAssistant *frontendAsistantWidget = qobject_cast<FrontendAssistant *>(currentWidget);
        message = frontendAsistantWidget ? frontendAsistantWidget->isValid() : "";
    } else if (currentIndex == 4) {
        BackendAssistant *backendAsistantWidget = qobject_cast<BackendAssistant *>(currentWidget);
        message = backendAsistantWidget ? backendAsistantWidget->isValid() : "";
    }

    if (message != "") {
        QMessageBox::warning(this, "Warning", QString::fromStdString(message));
        return; // Stop here
    }

    // Next step
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
