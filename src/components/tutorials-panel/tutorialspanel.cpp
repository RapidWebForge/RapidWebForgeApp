#include "tutorialspanel.h"
#include <QFile>
#include <QMessageBox>
#include <QScroller>
#include <QVBoxLayout>
#include "../../core/project-manager/projectmanager.h"
#include "../project-preview/projectpreview.h"
#include "../stepper-dashboard/stepperdashboard.h"
#include "ui_tutorialspanel.h"

TutorialsPanel::TutorialsPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TutorialsPanel)
{
    ui->setupUi(this);
    setupTutorials();
}

TutorialsPanel::~TutorialsPanel()
{
    delete ui;
}

void TutorialsPanel::setupTutorials()
{
    // Obtener los layouts de cada nivel desde el XML
    QGridLayout *gridBeginner = ui->scrollAreaWidgetContents->findChild<QGridLayout *>("gridBeginner");
    QGridLayout *gridIntermediate = ui->scrollAreaWidgetContents->findChild<QGridLayout *>("gridIntermediate");
    QGridLayout *gridAdvanced = ui->scrollAreaWidgetContents->findChild<QGridLayout *>("gridAdvanced");

    if (!gridBeginner || !gridIntermediate || !gridAdvanced)
        return;

    // Limpiar los layouts antes de agregar nuevos tutoriales
    auto clearLayout = [](QGridLayout *layout) {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->deleteLater();
            }
            delete child;
        }
    };

    clearLayout(gridBeginner);
    clearLayout(gridIntermediate);
    clearLayout(gridAdvanced);

    // Lista de tutoriales organizados por nivel
    QMap<QGridLayout *, QList<QPair<QString, QString>>> tutorialsByLevel;
    tutorialsByLevel[gridBeginner] = {
                                      {"HTML - Introducción a las etiquetas",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-1.json"},
                                      {"HTML - Atributos de las etiquetas",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-2.json"},
                                      {"CSS - Conociendo los estilos en cascada",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-3.json"},
                                      {"CSS - Diseño responsive",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-4.json"},
                                      {"React - Creando componentes",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-5.json"},
                                      {"React - Creando nuevas vistas",
                                       ":/resources/log_tutorials/begginer/begginer-tutorial-6.json"},
                                      };

    tutorialsByLevel[gridIntermediate]
        = {{"HTML: Insertar imágenes",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-1.json"},
           {"HTML: Crear enlaces",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-2.json"},
           {"Base de datos - Creando nuestros modelos",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-3.json"},
           {"Base de Datos - Relaciones SQL",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-4.json"},
           {"ORM - Entendiendo el ORM",
            ":/resources/log_tutorials/intermedium/intermediate-tutorial-5.json"}};

    tutorialsByLevel[gridAdvanced]
        = {{"React: Creando formularios",
            ":/resources/log_tutorials/advanced/advanced-tutorial-1.json"},
           {"React: Recuperando información",
            ":/resources/log_tutorials/advanced/advanced-tutorial-2.json"}};

    // Agregar tutoriales a cada nivel
    for (auto it = tutorialsByLevel.begin(); it != tutorialsByLevel.end(); ++it) {
        QGridLayout *grid = it.key();
        QList<QPair<QString, QString>> tutorials = it.value();

        int row = 0, col = 0;
        for (const auto &tutorial : tutorials) {
            QString tutorialTitle = tutorial.first;
            QString tutorialPath = tutorial.second;

            // Crear botón contenedor
            QPushButton *tutorialButton = new QPushButton(this);
            tutorialButton->setFixedSize(200, 122);
            tutorialButton->setStyleSheet(
                "QPushButton {"
                "   background-color: white;"
                "   border-radius: 10px;"
                "   border: 1px solid #ddd;"
                "   padding: 5px;"
                "} "
                "QPushButton:hover {"
                "   background-color: #f5f5f5;"
                "}");

            // Crear QLabel para el texto del botón
            QLabel *buttonLabel = new QLabel(tutorialTitle, tutorialButton);
            buttonLabel->setWordWrap(true);             // Permitir saltos de línea si es necesario
            buttonLabel->setAlignment(Qt::AlignCenter); // Centrar el texto
            buttonLabel->setStyleSheet("font-size: 14px; color: black; background-color: transparent;");

            // Crear un layout para el botón y agregar el QLabel dentro
            QVBoxLayout *buttonLayout = new QVBoxLayout(tutorialButton);
            buttonLayout->addWidget(buttonLabel);
            buttonLayout->setAlignment(Qt::AlignCenter);
            buttonLayout->setContentsMargins(5, 5, 5, 5);
            tutorialButton->setLayout(buttonLayout);

            connect(tutorialButton, &QPushButton::clicked, this, [this, tutorialPath]() {
                onTutorialClicked(tutorialPath);
            });

            grid->addWidget(tutorialButton, row, col);

            if (++col >= 3) { // Máximo 3 botones por fila
                col = 0;
                row++;
            }
        }
    }

    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Ajustar tamaño del contenedor
    ui->scrollAreaWidgetContents->adjustSize();
    ui->scrollArea->update();
}

void TutorialsPanel::onTutorialClicked(const QString &tutorialPath)
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
    qDebug() << "Opening tutorial with path: " << tutorialPath;
}
