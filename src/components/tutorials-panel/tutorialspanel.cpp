#include "tutorialspanel.h"
#include <QFile>
#include <QMessageBox>
#include <QScroller>
#include <QVBoxLayout>
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "ui_tutorialspanel.h"
#include <boost/process.hpp>
#include <string>

TutorialsPanel::TutorialsPanel(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TutorialsPanel)
{
    ui->setupUi(this);
    setupTutorials(); // ✅ Llamar a setupTutorials() en lugar de setupProjects()
}
void TutorialsPanel::setupTutorials()
{
    // Obtener el layout del scroll vertical
    QGridLayout *tutorialLayout = qobject_cast<QGridLayout *>(
        ui->scrollAreaWidgetContents->layout());

    if (!tutorialLayout)
        return;

    // Limpiar el layout antes de agregar nuevos tutoriales
    QLayoutItem *child;
    while ((child = tutorialLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }

    // Configurar el fondo del área de tutoriales como blanco
    ui->scrollAreaWidgetContents->setStyleSheet("background-color: white;");
    ui->scrollArea->setStyleSheet("background-color: white; border: none;");

    // Habilitar el desplazamiento táctil y con mouse en el área de tutoriales
    QScroller::grabGesture(ui->scrollArea, QScroller::LeftMouseButtonGesture);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Espaciado entre tutoriales
    tutorialLayout->setSpacing(15);
    tutorialLayout->setContentsMargins(20, 10, 20, 10);

    // Lista de tutoriales con sus rutas
    QMap<QString, QString> tutorials
        = {{"Beginner: HTML - Introducción a las etiquetas",
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

    int row = 0;
    for (auto it = tutorials.begin(); it != tutorials.end(); ++it) {
        // Crear un botón para cada tutorial
        QPushButton *tutorialButton = new QPushButton(this);
        tutorialButton->setFixedSize(400, 80); // Botón más grande para mejor legibilidad
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

        // Crear un QLabel dentro del botón para mostrar el título del tutorial con saltos de línea
        QLabel *label = new QLabel(it.key(), tutorialButton);
        label->setWordWrap(true);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            "font-size: 14px; color: black; padding: 5px; background-color: transparent;");

        // Crear un layout dentro del botón para organizar el texto
        QVBoxLayout *layout = new QVBoxLayout(tutorialButton);
        layout->addWidget(label);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setAlignment(Qt::AlignCenter);

        QString tutorialPath = it.value();
        int projectId
            = 1; // 📌 Aquí puedes definir un ID de proyecto por defecto o seleccionar uno dinámicamente

        // Conectar el botón con la señal `openTutorial()`
        connect(tutorialButton, &QPushButton::clicked, this, [this, tutorialPath, projectId]() {
            onTutorialClicked(tutorialPath, projectId);
        });

        tutorialLayout->addWidget(tutorialButton);
        row++;
    }

    // Ajustar tamaño del contenedor para permitir el scroll vertical
    int totalHeight = (tutorials.size() * 90)
                      + (tutorialLayout->spacing() * (tutorials.size() - 1));
    ui->scrollAreaWidgetContents->setMinimumHeight(totalHeight);
    ui->scrollAreaWidgetContents->setMaximumHeight(totalHeight);
}

void TutorialsPanel::onAddProjectClicked()
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
    connect(stepper, &Stepper::destroyed, this, &TutorialsPanel::show);

    connect(stepper, &Stepper::backToProjectsPanel, this, [this, stepper]() {
        stepper->close();
        this->show();
    });
}
void TutorialsPanel::onDeleteProjectRequested(int projectId)
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
        setupTutorials();

        qDebug() << "Proyecto con ID:" << projectId
                 << "ha sido eliminado exitosamente de la base de datos.";
    } else {
        qDebug() << "Eliminación cancelada para el proyecto con ID:" << projectId;
    }
}

void TutorialsPanel::onProjectPreviewClicked(const QString &tutorialPath, int projectId)
{
    qDebug() << "Opening tutorial with path: " << tutorialPath << " for project ID: " << projectId;

    // 📌 Buscar la ventana principal (ProjectsPanel) desde `TutorialsPanel`
    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }

    // 📌 OCULTAR `ProjectsPanel` completamente
    projectsPanel->hide();

    // 📌 Verificar si `projectId` es válido antes de proceder
    ProjectManager projectManager;
    std::optional<Project> projectOpt = projectManager.getProjectById(projectId);

    if (!projectOpt.has_value()) {
        QMessageBox::critical(this, "Error", "Invalid project ID.");
        projectsPanel->show();
        return;
    }

    // 📌 Extraer el valor del `std::optional<Project>`
    Project project = projectOpt.value();

    // 📌 Crear instancia de `StepperDashboard`, pasando el tutorialPath si es un tutorial
    StepperDashboard *stprDashboard = new StepperDashboard(nullptr, project, tutorialPath);
    stprDashboard->showMaximized();

    // 📌 Restaurar `ProjectsPanel` cuando `StepperDashboard` se cierre
    connect(stprDashboard, &StepperDashboard::destroyed, projectsPanel, [projectsPanel]() {
        projectsPanel->show();
    });
}

void TutorialsPanel::onTutorialClicked(const QString &tutorialPath, int projectId)
{
    // 📌 Buscar la ventana principal (ProjectsPanel) desde `TutorialsPanel`
    QWidget *projectsPanel = this;
    while (projectsPanel->parentWidget() != nullptr) {
        projectsPanel = projectsPanel->parentWidget();
    }

    // 📌 OCULTAR `ProjectsPanel` completamente
    projectsPanel->hide();

    // 📌 Verificar si `projectId` es válido antes de proceder
    ProjectManager projectManager;
    std::optional<Project> projectOpt = projectManager.getProjectById(projectId);

    if (!projectOpt.has_value()) {
        QMessageBox::critical(this, "Error", "Invalid project ID.");
        projectsPanel->show();
        return;
    }
    // 📌 Extraer el valor del `std::optional<Project>`
    Project project = projectOpt.value();

    // 📌 Crear instancia de `StepperDashboard`, pasando el `tutorialPath` como parámetro
    StepperDashboard *stprDashboard = new StepperDashboard(nullptr, project, tutorialPath);
    stprDashboard->showMaximized();

    // 📌 Restaurar `ProjectsPanel` cuando `StepperDashboard` se cierre
    connect(stprDashboard, &StepperDashboard::destroyed, projectsPanel, [projectsPanel]() {
        projectsPanel->show();
    });
    qDebug() << "Opening tutorial with path: " << tutorialPath << " for project: " << projectId;
}
TutorialsPanel::~TutorialsPanel()
{
    delete ui;
}
