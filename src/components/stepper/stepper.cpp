#include "stepper.h"
#include <QMessageBox>
#include "../../core/code-generator/codegenerator.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "ui_stepper.h"
#include <cstdlib>
#include <iostream>

Stepper::Stepper(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Stepper)
    , creationAssistant(new CreationAssistant())
    , databaseAssistant(new DatabaseAssistant())
    , frontendAssistant(new FrontendAssistant())
    , backendAssistant(new BackendAssistant())
    , summaryAssistant(new SummaryAssistant())
    , newProject()
    , projectManager()
{
    ui->setupUi(this);

    ui->stepsWidget->addWidget(creationAssistant);
    ui->stepsWidget->addWidget(databaseAssistant);
    ui->stepsWidget->addWidget(frontendAssistant);
    ui->stepsWidget->addWidget(backendAssistant);
    ui->stepsWidget->addWidget(summaryAssistant);
    applyStyles(); // Aplicar todos los estilos
    ui->stepsWidget->setCurrentWidget(creationAssistant);
}

Stepper::~Stepper()
{
    delete ui;
}

void Stepper::on_nextButton_clicked()
{
    // Get the current widget and index
    QWidget *currentWidget = ui->stepsWidget->currentWidget();
    int currentIndex = ui->stepsWidget->currentIndex();

    std::string message = "";

    // Check if exist isValid() method
    if (currentIndex == 0) {
        CreationAssistant *creationAssistantWidget = qobject_cast<CreationAssistant *>(
            currentWidget);
        message = creationAssistantWidget ? creationAssistantWidget->isValid(this->newProject) : "";
    } else if (currentIndex == 1) {
        DatabaseAssistant *databaseAsistantWidget = qobject_cast<DatabaseAssistant *>(currentWidget);
        message = databaseAsistantWidget ? databaseAsistantWidget->isValid(this->newProject) : "";
    } else if (currentIndex == 2) {
        FrontendAssistant *frontendAsistantWidget = qobject_cast<FrontendAssistant *>(currentWidget);
        message = frontendAsistantWidget ? frontendAsistantWidget->isValid(this->newProject) : "";
    } else if (currentIndex == 3) {
        BackendAssistant *backendAsistantWidget = qobject_cast<BackendAssistant *>(currentWidget);
        message = backendAsistantWidget ? backendAsistantWidget->isValid(this->newProject) : "";
        // Update SummaryAssistant before moving to the next step
        SummaryAssistant *summaryAssistantWidget = qobject_cast<SummaryAssistant *>(
            ui->stepsWidget->widget(currentIndex + 1));
        if (summaryAssistantWidget) {
            summaryAssistantWidget->setProjectInformation(this->newProject);
        }
    }

    if (message != "") {
        QMessageBox::warning(this, "Warning", QString::fromStdString(message));
        return; // Stop here
    }

    // Move to the next step
    if (currentIndex < ui->stepsWidget->count() - 1) {
        ui->stepsWidget->setCurrentIndex(currentIndex + 1);
    }

    // Create Project before Summary
    if (currentIndex == ui->stepsWidget->count() - 2) {
        // Create project in sqlite
        projectManager.createProject(this->newProject);

        // Inicializar repositorio Git si versions está habilitado
        if (this->newProject.getVersions()) {
            create_git_repo(this->newProject.getPath());
        }

        // Copy folder template to choose path
        CodeGenerator codeGenerator(this->newProject);
        codeGenerator.createBaseBackendProject();
        codeGenerator.createBaseFrontendProject();

        message = "Your project has been created successfully!";
        QMessageBox::information(this, "Successful", QString::fromStdString(message));
    }

    // Show dashboard
    if (currentIndex == ui->stepsWidget->count() - 1) {
        this->hide();

        // When a project is clicked, open the StepperDashboard for that project
        StepperDashboard *stprDashboard = new StepperDashboard(nullptr, this->newProject);
        stprDashboard->show();

        // Show when dashboard is closed
        connect(stprDashboard, &StepperDashboard::destroyed, this, &Stepper::show);
    }
}

void Stepper::on_backButton_clicked()
{
    int currentIndex = ui->stepsWidget->currentIndex();

    if (currentIndex > 0) {
        ui->stepsWidget->setCurrentIndex(currentIndex - 1);
    }
}

void Stepper::create_git_repo(const std::string &projectPath)
{
    std::string command = "cd " + projectPath + " && git init";
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "Git repository initialized successfully in: " << projectPath << std::endl;
    } else {
        std::cerr << "Failed to initialize Git repository." << std::endl;
    }
}

void Stepper::applyStyles()
{
    this->setStyleSheet("background-color: #ffffff;");
    QString generalStyle = "color: #333333; font-size: 14px; font-weight: normal;";
    QString inputStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                         "font-weight: normal; background-color: #ffffff; padding: 4px 8px;";

    ui->nextButton->setStyleSheet(
        "border: 1px solid #cccccc; border-radius: 7px; margin-left: 0px; padding: 6px 20px; "
        "font-weight: semi-bold;"
        "background-color: #0F66DE; color: #ffffff; font-size: 16px; margin-inline: 20px;");
    ui->backButton->setStyleSheet(
        "border: 1px solid #cccccc; border-radius: 7px; padding: 6px 20px; font-weight: semi-bold;"
        "background-color: #f5f5f5; color: #333333; font-size: 16px;");
}
