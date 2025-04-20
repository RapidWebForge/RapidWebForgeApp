#include "overviewpanel.h"
#include <QFile>
#include <QMessageBox>
#include <QScroller>
#include <QVBoxLayout>
#include "../../core/project-manager/projectmanager.h"
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "ui_overviewpanel.h"
#include <string>

OverviewPanel::OverviewPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OverviewPanel)
{
    ui->setupUi(this);

    // Modificar los labels desde el código
    ui->label_2->setText("Tutorials");
    ui->label_2->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; padding: 5px;");

    ui->label->setText("Recents");
    ui->label->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; padding: 5px;");
}
void OverviewPanel::setupProjects(const std::vector<Project> &projects)
{
    this->projects = projects;

    // Obtener el layout de tutoriales
    QHBoxLayout *tutorialLayout = qobject_cast<QHBoxLayout *>(
        ui->tutorialScrollAreaWidget->layout());
    if (!tutorialLayout)
        return;

    // Limpiar el layout de tutoriales antes de agregar nuevos
    QLayoutItem *child;
    while ((child = tutorialLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
    // Configurar el fondo del área de tutoriales como blanco
    ui->tutorialScrollAreaWidget->setStyleSheet("background-color: white;");
    ui->tutorialScrollArea->setStyleSheet("background-color: white; border: none;");

    // Habilitar el desplazamiento táctil y con mouse en el área de tutoriales
    QScroller::grabGesture(ui->tutorialScrollArea, QScroller::LeftMouseButtonGesture);
    // Espaciado entre botones de tutorial
    tutorialLayout->setSpacing(15);                     // Espacio entre los botones
    tutorialLayout->setContentsMargins(20, 10, 20, 10); // Márgenes del layout

    // Lista de tutoriales con sus rutas
    QMap<QString, QString> tutorials
        = {
           {"Beginner: HTML - Introducción a las etiquetas",
            ":/resources/log_tutorials/begginer/begginer-tutorial-1.json"},
           {"Beginner: HTML - Atributos de las etiquetas",
            ":/resources/log_tutorials/begginer/begginer-tutorial-2.json"},
           {"Beginner: CSS - Conociendo los estilos en cascada",
            ":/resources/log_tutorials/begginer/begginer-tutorial-3.json"},
           {"Beginner: CSS - Diseño responsive",
            ":/resources/log_tutorials/begginer/begginer-tutorial-4.json"},
           {"Beginner: React - Creando componentes",
            ":/resources/log_tutorials/begginer/begginer-tutorial-5.json"},
           {"Beginner: React - Creando nuevas vistas",
            ":/resources/log_tutorials/begginer/begginer-tutorial-6.json"},
           {"Intermediate: Base de datos - Creando nuestros modelos",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-1.json"},
           {"Intermediate: Base de Datos - Relaciones SQL",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-2.json"},
           {"Intermediate: ORM - Entendiendo el ORM",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-3.json"}};

    int row1 = 0;
    for (auto it = tutorials.begin(); it != tutorials.end(); ++it) {
        QPushButton *tutorialButton = new QPushButton(this);
        tutorialButton->setFixedSize(202, 122);
        tutorialButton->setStyleSheet("QPushButton {"
                                      "   background-color: white;"
                                      "   border-radius: 10px;"
                                      "   border: 1px solid #ddd;"
                                      "   box-shadow: 2px 2px 5px rgba(0, 0, 0, 0.1);"
                                      "   padding: 5px;"
                                      "} "
                                      "QPushButton:hover {"
                                      "   background-color: #f5f5f5;"
                                      "}");

        // Crear un QLabel dentro del botón para mostrar el texto correctamente
        QLabel *label = new QLabel(it.key(), tutorialButton);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            "font-size: 14px; color: black; padding: 5px; background-color: transparent;");

        // Crear layout y añadir el QLabel dentro del botón
        QVBoxLayout *layout = new QVBoxLayout(tutorialButton);
        layout->addWidget(label);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setAlignment(Qt::AlignCenter);

        QString tutorialPath = it.value();

        // Conectar el botón con la señal `openTutorial()`
        connect(tutorialButton, &QPushButton::clicked, this, [this, tutorialPath]() {
            onTutorialClicked(tutorialPath);
        });

        tutorialLayout->addWidget(tutorialButton);
        row1++;
    }
    QGridLayout *gridLayout = ui->gridLayout;
    gridLayout->setSpacing(10);

    // Limpiar el layout eliminando los widgets de manera segura
    while ((child = gridLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater(); // Elimina los widgets de forma asíncrona
        }
        delete child; // Borra el item del layout
    }
    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    connect(addProjectButton, &QPushButton::clicked, this, &OverviewPanel::onAddProjectClicked);

    // Crear el contenedor para el botón y el texto
    QWidget *newProjectWidget = new QWidget(this);
    QVBoxLayout *newProjectLayout = new QVBoxLayout(newProjectWidget);

    // Ajustar márgenes y espaciado para que el texto esté más cerca del botón
    newProjectLayout->setAlignment(Qt::AlignTop);
    newProjectLayout->setSpacing(5); // Ajusta el espaciado entre el botón y el texto
    newProjectLayout->setContentsMargins(0, 0, 0, 0);     // Quita márgenes adicionales
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
                &OverviewPanel::onProjectPreviewClicked);
        connect(projectPreview,
                &ProjectPreview::deleteRequested,
                this,
                &OverviewPanel::onDeleteProjectRequested);

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

void OverviewPanel::onAddProjectClicked()
{
    bool pathStatus = confManager->getConfiguration().getStatus();

    if (!pathStatus) {
        QMessageBox::critical(this, "Warning", "You need to set the tech paths in 'Configuration'");
        return;
    }

    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }
    projectsPanel->hide();

    // When the "+" button is clicked, open the Stepper window
    Stepper *stepper = new Stepper();
    stepper->show();

    // Show when create assistant is closed
    connect(stepper, &Stepper::destroyed, this, &OverviewPanel::show);

    connect(stepper, &Stepper::backToProjectsPanel, this, [this, stepper]() {
        stepper->close();
        this->show();
    });
}

void OverviewPanel::onDeleteProjectRequested(int projectId)
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

void OverviewPanel::onProjectPreviewClicked(const Project &project)
{
    // Buscar la ventana principal (ProjectsPanel) a partir de `OverviewPanel`
    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }

    // OCULTAR `ProjectsPanel` completamente
    projectsPanel->hide();

    // Abrir el `StepperDashboard` asegurando que tenga un `projectId` válido
    if (project.getId() == -1) {
        QMessageBox::critical(this, "Error", "Invalid project ID.");
        projectsPanel->show();
        return;
    }

    StepperDashboard *stprDashboard = new StepperDashboard(nullptr, project);
    stprDashboard->showMaximized();

    // Restaurar `ProjectsPanel` cuando `StepperDashboard` se cierre
    connect(stprDashboard, &StepperDashboard::destroyed, projectsPanel, [projectsPanel]() {
        projectsPanel->show();
    });
}

void OverviewPanel::onTutorialClicked(const QString &tutorialPath)
{
    // Buscar la ventana principal (ProjectsPanel) desde `TutorialsPanel`
    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }

    // OCULTAR `ProjectsPanel` completamente
    projectsPanel->hide();

    // Verificar si `projectId` es válido antes de proceder
    ProjectManager projectManager;
    std::optional<Project> projectOpt = projectManager.getProjectByName("Tutorial");

    if (!projectOpt.has_value()) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("Error");
        msgBox.setText("Tutorial project not found");
        msgBox.setInformativeText(
            "To use tutorials, please ensure you have a project named 'Tutorial'.");
        msgBox.setDetailedText(
            "The application couldn't find a project with the required name. "
            "Create a new project named 'Tutorial' or check your project settings.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();

        projectsPanel->show();
        return;
    }
    // Extraer el valor del `std::optional<Project>`
    Project project = projectOpt.value();

    // Crear instancia de `StepperDashboard`, pasando el `tutorialPath` como parámetro
    StepperDashboard *stprDashboard = new StepperDashboard(nullptr, project, tutorialPath);
    stprDashboard->showMaximized();

    // Restaurar `ProjectsPanel` cuando `StepperDashboard` se cierre
    connect(stprDashboard, &StepperDashboard::destroyed, projectsPanel, [projectsPanel]() {
        projectsPanel->show();
    });
}
OverviewPanel::~OverviewPanel()
{
    delete ui;
}
