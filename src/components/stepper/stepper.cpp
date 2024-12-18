#include "stepper.h"
#include <QCoreApplication>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMovie>
#include <QProgressDialog>
#include <QThread>
#include <QThreadPool>
#include <QVBoxLayout>
#include "../../core/code-generator/codegenerator.h"
#include "../customprogress-dialog/customprogressdialog.h"
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
        // Hide to avoid create the project twice
        ui->backButton->hide();
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
        // Crear y mostrar el diálogo personalizado
        CustomProgressDialog progressDialog(this);
        progressDialog.setWindowModality(Qt::WindowModal);
        progressDialog.show();

        // Procesar eventos para mostrar el diálogo
        QCoreApplication::processEvents();

        // Ejecutar tareas de creación de proyecto en segundo plano
        projectManager.createProject(this->newProject);

        // Copy folder template to choose path
        CodeGenerator codeGenerator(this->newProject);
        codeGenerator.createBaseBackendProject();
        codeGenerator.createBaseFrontendProject();

        // Cerrar el diálogo al finalizar las tareas
        progressDialog.close();

        QString message = "Your project has been created successfully!";
        QMessageBox::information(this, "Successful", message);

        // Inicializar repositorio Git si versions está habilitado
        if (this->newProject.getVersions()) {
            VersionManager versionManager(this->newProject.getPath());
            versionManager.initializeRepository();
        }
    }

    // Show dashboard
    if (currentIndex == ui->stepsWidget->count() - 1) {
        this->hide();

        // When a project is clicked, open the StepperDashboard for that project
        StepperDashboard *stprDashboard = new StepperDashboard(nullptr, this->newProject);
        stprDashboard->show();

        // Show when dashboard is closed
        connect(stprDashboard, &StepperDashboard::destroyed, this, &Stepper::show);

        // Realizar el commit inicial al pasar al summary o finalizar el proyecto
        if (this->newProject.getVersions()) {
            VersionManager versionManager(this->newProject.getPath());
            versionManager.saveChanges(); // Aquí se realiza el commit inicial
        }
    }
}

void Stepper::on_backButton_clicked()
{
    int currentIndex = ui->stepsWidget->currentIndex();

    if (currentIndex > 0) {
        ui->stepsWidget->setCurrentIndex(currentIndex - 1);
    } else if (currentIndex == 0) {
        emit backToProjectsPanel();
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
