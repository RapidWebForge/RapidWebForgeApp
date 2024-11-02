#include "projectspanel.h"
#include <QMessageBox>
#include "../../core/project-manager/projectmanager.h"
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "../stepper/stepper.h"
#include "ui_projectspanel.h"
#include <boost/process.hpp>
#include <string>

ProjectsPanel::ProjectsPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjectsPanel)
    , confManager()
{
    ui->setupUi(this);

    ProjectManager projectManager;
    setupProjects(projectManager.getAllProjects());
    applyStylesProj();
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
    addProjectButton->setFixedSize(190, 108); // Tamaño del botón, puedes ajustarlo según el diseño
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

    // Crear el contenedor para el botón y el texto
    QWidget *newProjectWidget = new QWidget(this);
    QVBoxLayout *newProjectLayout = new QVBoxLayout(newProjectWidget);

    // Ajustar márgenes y espaciado para que el texto esté más cerca del botón
    newProjectLayout->setAlignment(Qt::AlignTop);
    newProjectLayout->setSpacing(5); // Ajusta el espaciado entre el botón y el texto
    newProjectLayout->setContentsMargins(0, 0, 0, 0); // Quita márgenes adicionales
    newProjectLayout->setContentsMargins(10, 10, 10, 10); // Márgenes del contenedor
    newProjectWidget->setFixedSize(215, 180);             // Tamaño fijo para el contenedor

    // Agregar el botón y el texto "New" al contenedor
    newProjectLayout->addWidget(addProjectButton);

    QLabel *newLabel = new QLabel("New", this);
    newLabel->setAlignment(Qt::AlignCenter);
    newLabel->setStyleSheet(
        "font-size: 16px; color: #333; padding-top: 12px; "); // Personaliza el estilo del texto
    newProjectLayout->addWidget(newLabel);

    // Agregar el contenedor al layout principal
    gridLayout->addWidget(newProjectWidget, 0, 0);

    // Add projects to layout
    int row = 0;
    int column = 1;
    int maxColumns = 3;

    for (const auto &project : projects) {
        // Widget for each project
        ProjectPreview *projectPreview = new ProjectPreview(this, project);
        projectPreview->setFixedSize(215, 180); // Ajustar el tamaño de cada contenedor de proyecto

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

bool ProjectsPanel::checkCommand(const std::string &command, bool dobleQuote)
{
    namespace bp = boost::process;
    try {
        bp::ipstream is; // Stream para capturar la salida
        std::string version = dobleQuote ? " --version" : " -version";
        bp::child c(command + version, bp::std_out > is);
        std::string line;
        while (is && std::getline(is, line) && !line.empty()) {
            std::cout << line << std::endl; // Captura la salida
        }
        c.wait();
        return c.exit_code() == 0;
    } catch (...) {
        return false;
    }
}

void ProjectsPanel::onAddProjectClicked()
{
    std::vector<std::string> missingTech;
    std::string tech[4] = {
        confManager->getConfiguration().getNgInxPath(),
        confManager->getConfiguration().getnodePath(),
        confManager->getConfiguration().getBunPath(),
        confManager->getConfiguration().getMysqlPath(),
    };

    for (const auto &t : tech) {
        if (t.find("nginx") != std::string::npos) {
            if (!checkCommand(t, false)) {
                missingTech.push_back(t);
            }
        } else {
            if (!checkCommand(t)) {
                missingTech.push_back(t);
            }
        }
    }

    if (!missingTech.empty()) {
        std::string message = "You are missing the following tech:\n";
        for (const auto &tech : missingTech) {
            message += "- " + tech + "\n";
        }
        QMessageBox::critical(this, "Warning", QString::fromStdString(message));
        return;
    }

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
    StepperDashboard *stprDashboard = new StepperDashboard(nullptr, project);
    stprDashboard->showMaximized();

    // Show when dashboard is closed
    connect(stprDashboard, &StepperDashboard::destroyed, this, &ProjectsPanel::show);
}

ProjectsPanel::~ProjectsPanel()
{
    delete ui;
}

void ProjectsPanel::applyStylesProj()
{
    // Aplica el stylesheet a los elementos del panel
    QString stylesheet = R"(


        QWidget#widget {
            background-color: #f9f9f9;
        }

        QWidget#ProjectsPanel {
            background-color: white;
        }

        QPushButton#recentsButton{
            font-size: 14px;
            color: #000000;
            background-color: #ffffff;
            border-radius: 5px;
            padding: 7px 7px;
            text-align: left; /* Alineación del texto dentro del botón */
        }

        QPushButton#configurationButton {
            font-size: 14px;
            color: #000000;
            background-color: #ffffff;
            border: 0.5px solid #dddddd;
            border-radius: 5px;
            padding: 7px 7px;
        }


        QPushButton#recentsButton:hover, QPushButton#configurationButton:hover {
            background-color: #f0f0f0;
        }

        QPushButton#recentsButton:pressed, QPushButton#configurationButton:pressed {
            background-color: #e0e0e0;
        }

        QPushButton#addProjectButton {
            font-size: 36px;
            color: #666666;
            background-color: #f9f9f9;
            border: 2px dashed #cccccc;
            border-radius: 10px;
            min-width: 150px;
            min-height: 150px;
        }

        QPushButton#addProjectButton:hover {
            background-color: #e9e9e9;
        }

        QPushButton#addProjectButton:pressed {
            background-color: #d9d9d9;
        }

        QScrollArea {
            border: none;
            background-color: #ffffff;
        }

        QWidget#scrollAreaWidgetContents {
            background-color: #ffffff;
        }

        QLabel#label {
            font-size: 35px;
            font-weight: bold;
            color: #333333;
        }

        QWidget#projectPreview {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 10px;
            margin: 5px;
        }

        QWidget#projectPreview:hover {
            background-color: #f0f0f0;
        }

        QWidget#projectPreview:pressed {
            background-color: #e0e0e0;
        }

        QLabel#projectName {
            font-size: 18px;
            color: #333333;
        }
    )";

    this->setStyleSheet(stylesheet);
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
