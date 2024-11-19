#include "projectspanel.h"
#include <QFile>
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

    // Verificar el estado inicial
    bool pathStatus = confManager->getConfiguration().getStatus();
    if (!pathStatus) {
        QMessageBox::warning(this, "Warning", "You need to set the tech paths in 'Configuration'");
    }

    ProjectManager projectManager;
    setupProjects(projectManager.getAllProjects());    
    applyStylesProj();
}

void ProjectsPanel::setupProjects(const std::vector<Project> &projects)
{
    this->projects = projects;

    QGridLayout *gridLayout = ui->gridLayout;

    // Limpiar el layout eliminando los widgets de manera segura
    QLayoutItem *child;
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater(); // Elimina los widgets de forma asíncrona
        }
        delete child; // Borra el item del layout
    }

    // Crear el widget para agregar un nuevo proyecto
    QPushButton *addProjectButton = new QPushButton(this);
    addProjectButton->setText("+");
    addProjectButton->setFixedSize(202, 118); // Tamaño del botón, puedes ajustarlo según el diseño
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
        connect(projectPreview,
                &ProjectPreview::deleteRequested,
                this,
                &ProjectsPanel::onDeleteProjectRequested);

        gridLayout->addWidget(projectPreview, row, column);

        column++;
        if (column >= maxColumns) {
            column = 0;
            row++;
        }
    }
    // Ajuste del tamaño mínimo para que el scroll funcione correctamente
    ui->scrollAreaWidgetContents->adjustSize(); // Ajusta el tamaño del contenedor según el contenido

    ui->scrollAreaWidgetContents->setMinimumHeight(gridLayout->sizeHint().height());
    // Optionally set spacing and margins for better visual appearance
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10, 10, 10, 10);
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
}

bool ProjectsPanel::checkCommand(const std::string &command, bool dobleQuote)
{
    namespace bp = boost::process;
    try {
        std::string version = dobleQuote ? " --version" : " -version";
        bp::ipstream is; // Stream para capturar la salida

        // Ejecutar el comando en segundo plano y redirigir la salida a null
        bp::child c(command + version, bp::std_out > is, bp::std_err > bp::null);
        std::string line;
        while (std::getline(is, line) && !line.empty()) {
            std::cout << line << std::endl; // Opcional: para registro interno
        }
        c.wait();
        return c.exit_code() == 0;
    } catch (...) {
        return false;
    }
}

void ProjectsPanel::onAddProjectClicked()
{
    bool pathStatus = confManager->getConfiguration().getStatus();

    if (!pathStatus) {
        QMessageBox::critical(this, "Warning", "You need to set the tech paths in 'Configuration'");
        return;
    }

    this->hide();

    // When the "+" button is clicked, open the Stepper window
    Stepper *stepper = new Stepper();
    stepper->show();

    // Show when create assistant is closed
    connect(stepper, &Stepper::destroyed, this, &ProjectsPanel::show);

    connect(stepper, &Stepper::backToProjectsPanel, this, [this, stepper]() {
        stepper->close();
        this->show();
    });
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
void ProjectsPanel::onDeleteProjectRequested(int projectId)
{
    qDebug() << "Intentando eliminar el proyecto con ID:" << projectId;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Confirmar eliminación",
                                  "¿Estás seguro de que deseas eliminar este proyecto?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        qDebug() << "Confirmación recibida para eliminar el proyecto con ID:" << projectId;

        // Eliminar el proyecto de la base de datos sin recargar el QGridLayout
        ProjectManager projectManager;
        projectManager.deleteProjectById(projectId);
        // Aquí solo obtenemos los proyectos nuevamente sin eliminar en la base de datos
        setupProjects(projectManager.getAllProjects());

        qDebug() << "Proyecto con ID:" << projectId
                 << "ha sido eliminado exitosamente de la base de datos.";
    } else {
        qDebug() << "Eliminación cancelada para el proyecto con ID:" << projectId;
    }
}
