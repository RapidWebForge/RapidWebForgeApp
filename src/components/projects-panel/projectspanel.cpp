#include "projectspanel.h"
#include <QFile>
#include <QMessageBox>
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "../stepper/stepper.h"
#include "ui_projectspanel.h"
#include <string>

ProjectsPanel::ProjectsPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjectsPanel)
    , confManager()
    , overviewPanel(new OverviewPanel())
    , tutorialsPanel(new TutorialsPanel())
    , proPanel(new ProPanel())
{
    ui->setupUi(this);

    // Verificar el estado inicial
    bool pathStatus = confManager->getConfiguration().getStatus();
    if (!pathStatus) {
        QMessageBox::warning(this, "Warning", "You need to set the tech paths in 'Configuration'");
    }
    ui->stackedWidget->addWidget(overviewPanel);
    ui->stackedWidget->addWidget(tutorialsPanel);
    ui->stackedWidget->addWidget(proPanel);
    ui->stackedWidget->setCurrentWidget(proPanel);

    showRecents(); // Cambia a la pestaña "Overview"

    ui->stackedWidget->update();
    ui->stackedWidget->repaint();

    // Conectar botones con las funciones para cambiar de vista
    connect(ui->recentsButton, &QPushButton::clicked, this, &ProjectsPanel::showRecents);
    connect(ui->tutorialButton, &QPushButton::clicked, this, &ProjectsPanel::showTutorials);
    connect(ui->projectButton, &QPushButton::clicked, this, &ProjectsPanel::showProjects);

    applyStylesProj();
}

ProjectsPanel::~ProjectsPanel()
{
    delete ui;
}

void ProjectsPanel::applyStylesProj()
{
    QFile styleFile(":/styles/projectspanel");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        this->setStyleSheet(styleSheet);
    }
}

void ProjectsPanel::on_configurationButton_clicked()
{
    if (!configView) {
        configView = new ConfigurationView();
        configView->show();

        connect(configView, &ConfigurationView::finished, this, [this]() { configView = nullptr; });
    } else {
        configView->raise();
        configView->activateWindow();
    }
}

// Mostrar la página Overview (Recents)
void ProjectsPanel::showRecents()
{
    ui->stackedWidget->setCurrentWidget(overviewPanel);
    ui->label->setText("Overview");
    // Actualizar la lista de proyectos cada vez que abrimos la pestaña
    overviewPanel->setupProjects(this->projectManager.getAllProjects());
}

// Mostrar la página Tutorials
void ProjectsPanel::showTutorials()
{
    ui->stackedWidget->setCurrentWidget(tutorialsPanel);
    ui->label->setText("Tutorials");
    // Actualizar la lista de proyectos cada vez que abrimos la pestaña
    overviewPanel->setupProjects(this->projectManager.getAllProjects());
}

// Mostrar la página Projects
void ProjectsPanel::showProjects()
{
    ui->stackedWidget->setCurrentWidget(proPanel);
    ui->label->setText("Projects");
    // Actualizar la lista de proyectos cada vez que abrimos la pestaña
    proPanel->setupProjects(this->projectManager.getAllProjects());
}

