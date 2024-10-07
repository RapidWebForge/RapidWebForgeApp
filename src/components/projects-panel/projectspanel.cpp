#include "projectspanel.h"
#include <QMessageBox>
#include "../../core/project-manager/projectmanager.h"
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "../stepper/stepper.h"
#include "ui_projectspanel.h"

ProjectsPanel::ProjectsPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjectsPanel)
{
    ui->setupUi(this);

    ProjectManager projectManager;
    setupProjects(projectManager.getAllProjects());
}

void ProjectsPanel::setupProjects(const std::vector<Project> &projects)
{
    this->projects = projects;

    QGridLayout *gridLayout = ui->gridLayout;

    // Clean the layout
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }

    // Crear el widget para agregar un nuevo proyecto
    QPushButton *addProjectButton = new QPushButton(this);
    addProjectButton->setText("+");
    addProjectButton->setFixedSize(120, 120); // Tamaño del botón, puedes ajustarlo según el diseño
    addProjectButton->setStyleSheet("QPushButton {"
                                    "   font-size: 48px;"
                                    "   color: #555;"
                                    "   background-color: #f0f0f0;"
                                    "   border-radius: 10px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "   background-color: #e0e0e0;"
                                    "}");
    connect(addProjectButton, &QPushButton::clicked, this, &ProjectsPanel::onAddProjectClicked);

    gridLayout->addWidget(addProjectButton, 0, 0);

    // Add projects to layout
    int row = 0;
    int column = 1;
    int maxColumns = 3;

    for (const auto &project : projects) {
        // Widget for each project
        ProjectPreview *projectPreview = new ProjectPreview(this, project);

        // Connect the ProjectPreview click to open the StepperDashboard
        connect(projectPreview,
                &ProjectPreview::projectClicked,
                this,
                &ProjectsPanel::onProjectPreviewClicked);

        gridLayout->addWidget(projectPreview, row, column);

        column++;
        if (column >= maxColumns) {
            column = 0;
            row++;
        }
    }

    // Optionally set spacing and margins for better visual appearance
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
}

void ProjectsPanel::onAddProjectClicked()
{
    this->hide();

    // When the "+" button is clicked, open the Stepper window
    Stepper *stepper = new Stepper();
    stepper->show();

    // Show when create assistant is closed
    connect(stepper, &Stepper::destroyed, this, &ProjectsPanel::show);
}

void ProjectsPanel::onProjectPreviewClicked(const Project &project)
{
    this->hide();

    // When a project is clicked, open the StepperDashboard for that project
    StepperDashboard *stprDashboard = new StepperDashboard();
    stprDashboard->show();

    // Show when dashboard is closed
    connect(stprDashboard, &StepperDashboard::destroyed, this, &ProjectsPanel::show);
}

ProjectsPanel::~ProjectsPanel()
{
    delete ui;
}
