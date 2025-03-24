#include "stepperdashboard.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QTimer>
#include "../../components/create-version/createversion.h"
#include "../../components/delete-version/deleteversion.h"
#include "../../components/manage-version/manageversion.h"
#include "../../components/projects-panel/projectspanel.h"
#include "../../components/stepper/stepper.h"
#include "../../components/version-history/versionhistory.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/deploy-manager/deploymanager.h"
#include "../../core/version-manager/versionmanager.h"
#include "ui_stepperdashboard.h"
#include <fmt/core.h>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <QResizeEvent>

#include <QPushButton>
#include <QDebug>
#include <QIcon>
#include <QVBoxLayout>

nlohmann::json tutorialData;

StepperDashboard::StepperDashboard(QWidget *parent,
                                   const Project &project,
                                   const QString &tutorialPath)
    : QWidget(parent)
    , ui(new Ui::StepperDashboard)
    , frontendDashboard(new FrontendDashboard())
    , backendDashboard(new BackendDashboard())
    , projectMenu(new QMenu("Project", this))
    , versionsMenu(new QMenu("Versions", this))
    , projectChangeAction(new QAction("Change project", this))
    , createNewProjectAction(new QAction("Create new project", this))
    , saveChangesAction(new QAction("Save changes", this))
    , deployProjectAction(new QAction("Deploy project", this))
    , createVersionAction(new QAction("Create version", this))
    , changeVersionAction(new QAction("Change version", this))
    , versionHistoryAction(new QAction("Version history", this))
    , deleteVersionAction(new QAction("Delete version", this))
    , project(project)
    , codeGenerator(new CodeGenerator(project))
    , versionManager(new VersionManager(project.getPath()))
    , customTreeWidget(new CustomTreeWidget(nullptr)) // Se crea sin añadirse a la UI
    , tutorialFilePath(tutorialPath)
    , fileWatcher(new FileWatcher(this)) // Instancia de FileWatcher
    , floatingButton(nullptr)            // Inicializar como nullptr

{
    ui->setupUi(this);

    stepValidator = new StepValidator(tutorialFilePath.toStdString(),
                                      "resources/logs/user_actions.json");

    // Crear un temporizador para verificar el estado de los pasos cada 2 segundos
    QTimer *stepCheckTimer = new QTimer(this);
    connect(stepCheckTimer, &QTimer::timeout, this, [this]() {
        showStep(currentStepIndex); // Revisar el estado del paso actual
    });
    stepCheckTimer->start(2000);
    // Configurar los menús y acciones
    setupMenus();
    applyMenuStyles();

    ui->stackedWidget->addWidget(backendDashboard);
    ui->stackedWidget->addWidget(frontendDashboard);
    ui->stackedWidget->setCurrentWidget(backendDashboard);
    ui->goalLabel->setWordWrap(true); // Habilitar ajuste de línea
    ui->goalLabel->setSizePolicy(QSizePolicy::Expanding,
                                 QSizePolicy::Preferred); // Expansión horizontal
    // Conectar la señal de BackendDashboard para que se guarden los cambios
    connect(backendDashboard,
            &BackendDashboard::transactionNameChanged,
            this,
            &StepperDashboard::onSaveChanges);

    // Conectar los botones a los slots
    connect(ui->backendButton, &QPushButton::clicked, this, &StepperDashboard::showBackendPage);
    connect(ui->frontendButton, &QPushButton::clicked, this, &StepperDashboard::showFrontendPage);
    connect(ui->commentButton, &QPushButton::clicked, this, &StepperDashboard::showTutorialComment);
    connect(ui->helpButton, &QPushButton::clicked, this, &StepperDashboard::showTutorialHelp);


    connect(customTreeWidget,
            &CustomTreeWidget::stepUpdated,
            this,
            &StepperDashboard::validateCurrentStep,
            Qt::QueuedConnection);

    // Asignar los menús a los botones
    ui->projectButton->setMenu(projectMenu);
    ui->versionsButton->setMenu(versionsMenu);

    connect(this,
            &StepperDashboard::backendSchemaLoaded,
            this,
            &StepperDashboard::onBackendSchemaLoaded);
    connect(this,
            &StepperDashboard::frontendSchemaLoaded,
            this,
            &StepperDashboard::onFrontendSchemaLoaded);

    qDebug() << "StepperDashboard constructor called.";
    qDebug() << "tutorialFilePath received: " << tutorialPath;

    if (tutorialPath.isEmpty()) {
        qDebug() << "❌ tutorialPath is EMPTY! Project mode activated.";
        isTutorialMode = false;
    } else {
        qDebug() << "tutorialPath is NOT empty! Tutorial mode activated.";
        isTutorialMode = true;
    }
    if (!isTutorialMode) {
        qDebug() << "❌ Hiding tutorial bar for project mode.";
        for (int i = 0; i < ui->tutorialBar->count(); ++i) {
            QLayoutItem *item = ui->tutorialBar->itemAt(i);
            if (item && item->widget()) {
                item->widget()->setVisible(false);
            }
        }
    } else {
        qDebug() << "Showing tutorial bar for tutorial mode.";
        for (int i = 0; i < ui->tutorialBar->count(); ++i) {
            QLayoutItem *item = ui->tutorialBar->itemAt(i);
            if (item && item->widget()) {
                item->widget()->setVisible(true);
            }
        }
    }
    // Mostrar el primer paso del primer tutorial
    showStep(0);
    // Configurar el entorno según el tipo de apertura

    // Muestra la barra de tutoriales solo si la opción de tutoriales está activa
    // Configurar la barra de tutoriales solo si el modo tutorial está activo
    setupTutorialConnections();
    if (isTutorialMode) {
        qDebug() << "Tutorial Mode Activated: Loading tutorial from " << tutorialFilePath;
        loadTutorialData();
        initializeTutorialBar();
    } else {
        qDebug() << "Project Mode Activated";
    }
    OverviewPanel *overviewPanel = new OverviewPanel();
    qDebug() << "StepperDashboard constructor called.";
    qDebug() << "tutorialFilePath received: " << tutorialFilePath;

    // Conectar la señal `openTutorial` con el método `loadTutorialData`
    connect(overviewPanel, &OverviewPanel::openTutorial, this, &StepperDashboard::loadTutorialData);

    setupFloatingButton();
    // Conectar el botón con la función que abre el archivo en VS Code
    connect(floatingButton, &QPushButton::clicked, this, &StepperDashboard::openLastModifiedFile);

    // Configurar FileWatcher para monitorear cambios en archivos del proyecto
    fileWatcher->watchProjectFiles(QString::fromStdString(project.getPath()));

    QString projectPath = QString::fromStdString(project.getPath());
    qDebug() << "📂 Ruta asignada para monitoreo: " << projectPath;

    fileWatcher->watchProjectFiles(projectPath);
}

void StepperDashboard::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    QTimer::singleShot(0, this, [this]() {
        bool frontendOk = false, backendOk = false;

        // Backend
        if (codeGenerator->backendGenerator.loadSchema()) {
            // QMessageBox::information(this, "Successful", "Information loaded");
            backendOk = true;

            emit backendSchemaLoaded();
        } else {
            qDebug() << "There is no backend content";
            // QMessageBox::warning(this, "Warning", "There is no information, add data");
        }
        // Frontend
        if (codeGenerator->frontendGenerator.loadSchema()) {
            // QMessageBox::information(this, "Successful", "Views loaded");
            frontendOk = true;

            emit frontendSchemaLoaded();
        } else {
            qDebug() << "There is no frontend content";
            // QMessageBox::warning(this, "Warning", "There is no views, add visual content");
        }

        QMessageBox::information(this,
                                 "Successful",
                                 frontendOk && backendOk ? "Contend loaded"
                                                         : "There was not content to load");
    });
}

void StepperDashboard::onBackendSchemaLoaded()
{
    std::vector<Transaction> transactions = codeGenerator->backendGenerator.getTransactions();
    backendDashboard->setTransactions(transactions);

    if (!transactions.empty()) {
        backendDashboard->setCurrentTransaction(transactions.at(0));
    }

    backendDashboard->setDatabaseLabel(project.getDatabaseData().getDatabaseName());
}

void StepperDashboard::onFrontendSchemaLoaded()
{
    auto frontendRoot = codeGenerator->frontendGenerator.getFrontendRoot();

    frontendDashboard->setFrontendRoot(frontendRoot);

    // Called here once the custom components vector is fill
    frontendDashboard->fillAvailableSections();
    frontendDashboard->addCustomComponentsOnComponentsTree();

    auto viewsNode = codeGenerator->frontendGenerator.getMainNode("Views");

    if (viewsNode) {
        // Verificar si tiene vistas disponibles
        auto views = viewsNode->getChildren();
        if (!views.empty()) {
            // Seleccionar la primera vista y configurarla como la sección actual
            auto firstView = std::dynamic_pointer_cast<Section>(views.at(0));
            if (firstView) {
                frontendDashboard->setCurrentSection(firstView);
            }
        }
    }
}

StepperDashboard::~StepperDashboard()
{
    if (ui) {
        delete ui;
    }
}

// Slot para mostrar la vista de Backend
void StepperDashboard::showBackendPage()
{
    ui->stackedWidget->setCurrentWidget(backendDashboard);

    // Aplicar estilos cuando se seleccione el Backend
    ui->backendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #0F66DE;" // Color azul para el botón seleccionado
        "   color: white;"
        "   border-radius: 5px;"
        "   padding: 4px 30px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004494;"
        "}");

    // Restablecer el estilo del botón de Frontend
    ui->frontendButton->setStyleSheet("QPushButton {"
                                      "   background-color: white;"
                                      "   color: #333;"
                                      "   border-radius: 5px;"
                                      "   padding: 4px 30px;"
                                      "   font-size: 14px;"
                                      "}"
                                      "QPushButton:hover {"
                                      "   background-color: #eaeaea;"
                                      "}"
                                      "QPushButton:pressed {"
                                      "   background-color: #dddddd;"
                                      "}");
}

// Slot para mostrar la vista de Frontend
void StepperDashboard::showFrontendPage()
{
    // Cambiar la vista al frontend
    ui->stackedWidget->setCurrentWidget(frontendDashboard);

    // Aplicar estilos cuando se seleccione el Frontend
    ui->frontendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #0F66DE;" // Color azul para el botón seleccionado
        "   color: white;"
        "   border-radius: 5px;"
        "   padding: 4px 30px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #0056b3;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #004494;"
        "}");

    // Restablecer el estilo del botón de Backend
    ui->backendButton->setStyleSheet("QPushButton {"
                                     "   background-color: white;"
                                     "   color: #333;"
                                     "   border-radius: 5px;"
                                     "   padding: 4px 30px;"
                                     "   font-size: 14px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "   background-color: #eaeaea;"
                                     "}"
                                     "QPushButton:pressed {"
                                     "   background-color: #dddddd;"
                                     "}");
}

void StepperDashboard::applyMenuStyles()
{
    this->setStyleSheet("background-color: #ffffff;");

    // Aplica estilos a los botones del top bar
    ui->projectButton->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   color: #333;"
        "   border-radius: 5px;"
        "   margin: 0px 5px;"
        "   padding: 4px 20px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #eaeaea;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #dddddd;"
        "}"
        "QPushButton::menu-indicator {"
        "   subcontrol-position: right center;" // Alinea el indicador (flecha) a la derecha
        "   subcontrol-origin: padding;"
        "}");

    // Aplica estilos al menú desplegable de Project
    ui->versionsButton->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   color: #333;"
        "   border-radius: 5px;"
        "   margin: 0px 5px;"
        "   padding: 4px 20px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #eaeaea;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #dddddd;"
        "}"
        "QPushButton::menu-indicator {"
        "   subcontrol-position: right center;" // Alinea el indicador (flecha) a la derecha
        "   subcontrol-origin: padding;"
        "}");

    // Estilo para los botones de Backend y Frontend
    ui->backendButton->setStyleSheet("QPushButton {"
                                     "   background-color: #0F66DE;"
                                     "   color: white;"
                                     "   border-radius: 5px;"
                                     "   padding: 4px 30px;"
                                     "   font-size: 14px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "   background-color: #0056b3;"
                                     "}"
                                     "QPushButton:pressed {"
                                     "   background-color: #004494;"
                                     "}");

    ui->frontendButton->setStyleSheet("QPushButton {"
                                      "   background-color: white;"
                                      "   color: #333;"
                                      "   border-radius: 5px;"
                                      "   padding: 4px 30px;"
                                      "   font-size: 14px;"
                                      "}"
                                      "QPushButton:hover {"
                                      "   background-color: #eaeaea;"
                                      "}"
                                      "QPushButton:pressed {"
                                      "   background-color: #dddddd;"
                                      "}");

    // Estilo para los menús desplegables
    projectMenu->setStyleSheet(
        "QMenu {"
        "   background-color: rgba(255, 255, 255, 230);" // Fondo translúcido (efecto bokeh)
        "   border-radius: 8px;"
        "   padding: 5px;"
        "   margin:10px;"
        "}"
        "QMenu::item {"
        "   background-color: rgba(255, 255, 255, 10);" // Mismo color de fondo para cada item del menú
        "   padding: 8px 16px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #d6d6d6;"
        "   color: #333;"
        "}"
        "QMenu::separator {"
        "   height: 1px;"
        "   background: #e5e5e5;"
        "   margin: 5px 0;"
        "}");

    versionsMenu->setStyleSheet(
        "QMenu {"
        "   background-color: rgba(255, 255, 255, 230);" // Fondo translúcido (efecto bokeh)
        "   border-radius: 12px;"
        "   padding: 5px;"
        "   margin:10px;"
        "}"
        "QMenu::item {"
        "   background-color: rgba(255, 255, 255, 10);" // Fondo translúcido (efecto bokeh)
        "   padding: 8px 16px;"
        "}"
        "QMenu::item:selected {"
        "   background-color: #d6d6d6;"
        "   color: #333;"
        "}"
        "QMenu::separator {"
        "   height: 1px;"
        "   background: #e5e5e5;"
        "   margin: 5px 0;"
        "}");

    // Estilo para el label de "GOAL"
    ui->goalLabel->setStyleSheet("QLabel {"
                                 "   color: #333;"        // Texto oscuro
                                 "   font-size: 14px;"    // Tamaño de fuente
                                 "   font-weight: bold;"  // Texto en negrita
                                 "   margin-right: 20px;" // Separación con los botones
                                 "}");

    // Estilo para los botones (comentarios, ayuda, siguiente)
    ui->commentButton->setStyleSheet("QPushButton {"
                                     "   background-color: #ffffff;" // Fondo blanco
                                     "   color: #1e90ff;"            // Texto azul
                                     "   border: 0px solid #1e90ff;" // Borde azul
                                     "   border-radius: 20px;"       // Forma circular
                                     "   width: 40px;"               // Ancho fijo
                                     "   height: 40px;"              // Alto fijo
                                     "   font-size: 16px;"           // Tamaño de fuente
                                     "   font-weight: bold;"         // Texto en negrita
                                     "} "
                                     "QPushButton:hover {"
                                     "   background-color: #e9e9e9;" // Azul claro al pasar el cursor
                                     "} "
                                     "QPushButton:pressed {"
                                     "   background-color: #d4ebff;" // Azul más oscuro al presionar
                                     "}");

    ui->helpButton->setStyleSheet("QPushButton {"
                                  "   background-color: #ffffff;" // Fondo blanco
                                  "   color: #ff0000;"            // Texto azul
                                  "   border: 0px solid #1e90ff;" // Borde azul
                                  "   border-radius: 20px;"       // Forma circular
                                  "   width: 40px;"               // Ancho fijo
                                  "   height: 40px;"              // Alto fijo
                                  "   font-size: 16px;"           // Tamaño de fuente
                                  "   font-weight: bold;"         // Texto en negrita
                                  "} "
                                  "QPushButton:hover {"
                                  "   background-color: #e9e9e9;" // Azul claro al pasar el cursor
                                  "} "
                                  "QPushButton:pressed {"
                                  "   background-color: #d4ebff;" // Azul más oscuro al presionar
                                  "}");

    ui->nextStepButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #28a745;" // Fondo verde
        "   color: white;"              // Texto blanco
        "   border: none;"              // Sin bordes
        "   border-radius: 8px;"        // Bordes redondeados
        "   padding: 8px 20px;"         // Espaciado interno
        "   font-size: 16px;"           // Tamaño de fuente
        "   font-weight: bold;"         // Texto en negrita
        "} "
        "QPushButton:hover {"
        "   background-color: #218838;" // Verde más oscuro al pasar el cursor
        "} "
        "QPushButton:pressed {"
        "   background-color: #1e7e34;" // Verde aún más oscuro al presionar
        "}");

    ui->goalLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   color: #333333;"      // Color del texto
        "   padding-right: 10px;" // Espacio interno para separarlo de los botones
        "}");
}

void StepperDashboard::setupMenus()
{
    // Configurar acciones para el menú de Project
    projectMenu->addAction(projectChangeAction);
    projectMenu->addAction(createNewProjectAction);
    projectMenu->addAction(saveChangesAction);
    projectMenu->addSeparator(); // Añadir un separador
    projectMenu->addAction(deployProjectAction);

    // Configurar acciones para el menú de Versions
    versionsMenu->addAction(createVersionAction);
    versionsMenu->addAction(changeVersionAction);
    versionsMenu->addAction(versionHistoryAction);
    versionsMenu->addSeparator(); // Añadir un separador
    versionsMenu->addAction(deleteVersionAction);

    // Configurar las acciones de cada opción

    connect(projectChangeAction, &QAction::triggered, this, &StepperDashboard::onProjectChange);

    // Conectar señales de las acciones a slots si es necesario
    connect(createNewProjectAction, &QAction::triggered, this, &StepperDashboard::onCreateProject);
    connect(saveChangesAction, &QAction::triggered, this, &StepperDashboard::onSaveChanges);
    connect(deployProjectAction, &QAction::triggered, this, &StepperDashboard::onDeployProject);
    // Versions
    connect(createVersionAction, &QAction::triggered, this, &StepperDashboard::onCreateVersion);
    connect(changeVersionAction, &QAction::triggered, this, &StepperDashboard::onChangeVersion);
    connect(versionHistoryAction, &QAction::triggered, this, &StepperDashboard::onVersionHistory);
    connect(deleteVersionAction, &QAction::triggered, this, &StepperDashboard::onDeleteVersion);
}

bool StepperDashboard::showConfirmationDialog(QWidget *parent,
                                              const QString &title,
                                              const QString &message)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent,
                                  title,
                                  message,
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);
    return (reply == QMessageBox::Yes);
}

void StepperDashboard::onSaveChanges()
{
    codeGenerator->backendGenerator.setTransactions(backendDashboard->getTransactions());

    codeGenerator->backendGenerator.updateBackendCode();

    // TODO: PASS AST UPDATE

    if (codeGenerator->frontendGenerator.updateFrontendCode()) {
        QMessageBox::information(this, "Save Changes", "Changes have been saved successfully.");
    } else {
        QMessageBox::warning(this, "Failed", "Failed to update JSON and generate code.");
    }
}

void StepperDashboard::onCreateVersion()
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to continue?")) {
            return;
        }
    }

    // Mostrar el diálogo para ingresar el nombre de la versión
    CreateVersion dialog(versionManager, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Obtener el nombre de la versión del diálogo
        QString versionName = dialog.getVersionName();

        if (versionName.isEmpty()) {
            QMessageBox::warning(this, "Invalid Version", "Version name cannot be empty.");
            return;
        }

        // Crear la versión en el repositorio (crear una nueva rama)
        versionManager->createVersion(versionName.toStdString());

        // Confirmación de éxito
        QMessageBox::information(this, "Success", "Version created successfully.");
    }
}

void StepperDashboard::onChangeVersion()
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to continue?")) {
            return;
        }
    }

    // Crear el diálogo y pasar el `versionManager`
    ManageVersion dialog(versionManager, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Obtener la rama seleccionada del diálogo
        QString selectedBranch = dialog.getSelectedBranch();
        if (selectedBranch.isEmpty()) {
            QMessageBox::warning(this, "Change Version", "No branch selected.");
            return;
        }

        // Cambiar a la rama seleccionada
        versionManager->changeVersion(selectedBranch.toStdString());

        // Confirmación de éxito
        QMessageBox::information(this, "Change Version", "Switched to version: " + selectedBranch);
    }
    // TODO: Usar el version manager
}

void StepperDashboard::onDeleteVersion()
{
    // Crear el diálogo para eliminar versiones
    DeleteVersion dialog(this);

    // Obtener la lista de versiones y establecerlas en el diálogo
    std::vector<std::string> versions = versionManager->listVersions();
    dialog.setVersions(versions);

    if (dialog.exec() == QDialog::Accepted) {
        // Obtener la versión seleccionada
        QString selectedVersion = dialog.getSelectedVersion();

        if (selectedVersion.isEmpty()) {
            QMessageBox::warning(this, "Delete Version", "No version selected.");
            return;
        }

        // Eliminar la versión seleccionada
        versionManager->deleteVersion(selectedVersion.toStdString());

        // Confirmación de éxito
        QMessageBox::information(this,
                                 "Delete Version",
                                 "Version '" + selectedVersion + "' deleted successfully.");
    }
}

void StepperDashboard::onVersionHistory()
{
    VersionHistory dialog(this);

    // Obtener el historial de commits y ramas
    std::vector<std::string> commits = versionManager->listCommits();
    std::vector<std::string> branches = versionManager->listVersions();

    // Verificar que la lista de commits no esté vacía
    if (commits.empty() && branches.empty()) {
        QMessageBox::information(this, "Version History", "No commits or branches found.");
        return;
    }

    // Establecer la lista de commits y ramas en el diálogo
    dialog.setCommits(commits);
    dialog.setBranches(branches);

    // Mostrar el diálogo
    dialog.exec();
    // TODO: Usar el version manager
}

void StepperDashboard::onDeployProject()
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to continue?")) {
            return;
        }
    }

    std::vector<Transaction> transactions = codeGenerator->backendGenerator.getTransactions();

    if (transactions.empty()) {
        QMessageBox::critical(this, "Critical", "You cannot deploy without generate transactions");
        return;
    }

    ConfigurationManager configurationManager;

    std::string ngInxPath = configurationManager.getConfiguration().getNgInxPath();
    std::string bunPath = configurationManager.getConfiguration().getBunPath();

    if (ngInxPath.empty()) {
        QMessageBox::critical(this, "Critical", "You cannot deploy without set NgInx Path");
        return;
    }
    if (bunPath.empty()) {
        QMessageBox::critical(this, "Critical", "You cannot deploy without set bun Path");
        return;
    }

    try {
        DeployManager deployManager(project.getPath(), ngInxPath);
        // Iniciar el despliegue
        deployManager.start(bunPath);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Critical Error", e.what());
        return;
    }
}

void StepperDashboard::onProjectChange()
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to continue?")) {
            return;
        }
    }

    ConfigurationManager configurationManager;
    // Detener Nginx al cerrar el proyecto
    try {
        DeployManager deployManager(project.getPath(),
                                    configurationManager.getConfiguration().getNgInxPath());
        deployManager.kill();
    } catch (const std::exception &e) {
        QMessageBox::warning(this,
                             "Warning",
                             "Failed to stop Nginx: " + QString::fromStdString(e.what()));
    }

    // Cerrar el StepperDashboard
    this->close();

    // Crear y mostrar el ProjectsPanel
    ProjectsPanel *projectsPanel = new ProjectsPanel();
    projectsPanel->setAttribute(Qt::WA_DeleteOnClose); // Liberar memoria automáticamente al cerrar
    projectsPanel->show();
}

void StepperDashboard::onCreateProject()
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to continue?")) {
            return;
        }
    }

    // Cerrar el StepperDashboard
    this->close();

    // Crear y mostrar el ProjectsPanel
    Stepper *createProjects = new Stepper();
    createProjects->setAttribute(Qt::WA_DeleteOnClose); // Liberar memoria automáticamente al cerrar
    createProjects->show();
}

void StepperDashboard::initializeTutorialBar()
{
    // Verifica si estamos en modo tutorial o no
    if (!isTutorialMode) {
        // Ocultar todos los widgets dentro del tutorialBar
        for (int i = 0; i < ui->tutorialBar->count(); ++i) {
            QLayoutItem *item = ui->tutorialBar->itemAt(i);
            if (item && item->widget()) {
                item->widget()->setVisible(false);
            }
        }
        return;
    }

    // Si es modo tutorial, mostrar los widgets de la barra
    for (int i = 0; i < ui->tutorialBar->count(); ++i) {
        QLayoutItem *item = ui->tutorialBar->itemAt(i);
        if (item && item->widget()) {
            item->widget()->setVisible(true);
        }
    }

    // Conectar botones de la barra de tutoriales
    setupTutorialConnections();
}

void StepperDashboard::setupTutorialConnections()
{
    // Desconectar cualquier conexión previa para evitar ejecuciones múltiples
    disconnect(ui->nextStepButton, nullptr, this, nullptr);

    // Conectar el botón Next correctamente a goToNextTutorialStep()
    connect(ui->nextStepButton,
            &QPushButton::clicked,
            this,
            &StepperDashboard::goToNextTutorialStep);
}

void StepperDashboard::showTutorialComment()
{
    QString comment = ui->commentButton->toolTip(); // Obtener el comentario actual

    if (comment.isEmpty()) {
        QMessageBox::warning(this, "Comment", "No comment available.");
        return;
    }

    // Crear cuadro de diálogo personalizado
    QDialog dialog(this);
    dialog.setWindowTitle("Comment");
    dialog.setMinimumSize(400, 200);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Crear etiqueta para el comentario
    QLabel *commentLabel = new QLabel(comment, &dialog);
    commentLabel->setWordWrap(true);
    layout->addWidget(commentLabel);

    // Agregar el enlace si hay una referencia
    if (!currentReference.isEmpty()) {
        QLabel *linkLabel = new QLabel("<a href=\"" + currentReference + "\">More Info</a>",
                                       &dialog);
        linkLabel->setOpenExternalLinks(true); // Permite abrir el enlace en el navegador
        layout->addWidget(linkLabel);
    }

    // Botón de cierre
    QPushButton *closeButton = new QPushButton("Close", &dialog);
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(closeButton);

    dialog.exec(); // Mostrar diálogo
}

void StepperDashboard::showTutorialHelp()
{
    QString help = ui->helpButton->toolTip(); // Obtener el tooltip asignado en showStep()

    if (!help.isEmpty()) {
        QMessageBox::information(this, "Help", help);
    } else {
        QMessageBox::warning(this, "Help", "No help available.");
    }
}

void StepperDashboard::goToNextTutorialStep()
{
    static bool tutorialCompleted = false; // Variable de control para evitar ejecución doble

    if (tutorialCompleted) {
        return; // Si ya se ejecutó una vez, no hacer nada
    }
    // Validar si el usuario ha completado el paso actual antes de permitir avanzar
    QJsonObject step = tutorialSteps[currentStepIndex].toObject();
    QString logAction = step["log"].toString();
    bool isCompleted = stepValidator->isStepCompleted(logAction.toStdString(), "");

    if (!isCompleted) {
        qDebug() << "❌ Step " << currentStepIndex
                 << " not completed. Next button remains disabled.";
        return; // No avanza hasta que el paso se complete
    }

    // Si hay más pasos, avanzar al siguiente
    if (currentStepIndex < tutorialSteps.size() - 1) {
        currentStepIndex++;
        showStep(currentStepIndex);
    }
    // Si ya estamos en el último paso y el usuario lo ha completado, mostrar el mensaje de finalización
    else if (currentStepIndex == tutorialSteps.size() - 1) {
        tutorialCompleted = true; // Marcar como completado para evitar ejecución doble

        QMessageBox::information(this, "Tutorial", "You have completed all steps.");

        // Cerrar el tutorial
        this->close();
        // Abrir el ProjectsPanel después de cerrar el StepperDashboard
        ProjectsPanel *projectsPanel = new ProjectsPanel();
        projectsPanel->setAttribute(
            Qt::WA_DeleteOnClose); // Liberar memoria automáticamente al cerrar
        projectsPanel->show();
    }
}

void StepperDashboard::loadTutorialData()
{
    qDebug() << "Loading tutorial data...";
    if (tutorialFilePath.isEmpty()) {
        QMessageBox::critical(this, "Error", "No tutorial file path provided.");
        return;
    }

    qDebug() << "Opening tutorial file from path: " << tutorialFilePath;

    QFile tutorialFile(tutorialFilePath);
    if (!tutorialFile.open(QIODevice::ReadOnly)) {
        qDebug() << "❌ Could not open tutorial file at path:" << tutorialFilePath;
        QMessageBox::critical(this, "Error", "Could not open the tutorial JSON file.");
        return;
    } else {
        qDebug() << "Tutorial file opened successfully.";
    }

    QByteArray data = tutorialFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    tutorialFile.close();
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "❌ Invalid tutorial JSON format!";
        QMessageBox::critical(this, "Error", "Invalid tutorial JSON format.");
        return;
    } else {
        qDebug() << "Tutorial JSON loaded correctly.";
    }
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        QMessageBox::critical(this, "Error", "Invalid tutorial JSON format.");
        return;
    }

    QJsonObject rootObj = jsonDoc.object();
    qDebug() << "JSON Object Keys: " << rootObj.keys();
    if (!rootObj.contains("steps")) {
        qDebug() << "Error: JSON does not contain 'steps'";
    } else if (!rootObj["steps"].isArray()) {
        qDebug() << "Error: 'steps' is not an array";
    } else {
        tutorialSteps = rootObj["steps"].toArray();
        qDebug() << "Total Steps Loaded: " << tutorialSteps.size();
    }

    qDebug() << "Tutorial Title: " << rootObj["title"].toString();
    qDebug() << "Tutorial Description: " << rootObj["description"].toString();
    qDebug() << "Steps present: " << rootObj["steps"].isArray();

    if (!rootObj.contains("steps") || !rootObj["steps"].isArray()) {
        QMessageBox::critical(this, "Error", "Invalid JSON: 'steps' is missing or not an array.");
        return;
    }
    tutorialTitle = rootObj["title"].toString();
    tutorialDescription = rootObj["description"].toString();

    if (rootObj.contains("steps") && rootObj["steps"].isArray()) {
        tutorialSteps = rootObj["steps"].toArray();
        qDebug() << "Loaded tutorial steps. Steps count:" << tutorialSteps.size();
    } else {
        qDebug() << "❌ 'steps' is missing or not an array.";
        QMessageBox::critical(this, "Error", "Invalid JSON: 'steps' is missing or not an array.");
        return;
    }
    if (!tutorialSteps.isEmpty()) {
        qDebug() << "Showing first step...";
        showStep(0);
    } else {
        QMessageBox::critical(this, "Error", "No tutorial steps found.");
    }
    qDebug() << "Total Steps Loaded: " << tutorialSteps.size();

    for (int i = 0; i < tutorialSteps.size(); ++i) {
        QJsonObject step = tutorialSteps[i].toObject();
        qDebug() << "Step " << i << " Goal: " << step["goal"].toString();
        qDebug() << "Step " << i << " Comment: " << step["comment"].toString();
        qDebug() << "Step " << i << " Help: " << step["help"].toString();
    }

    showTutorialIntro();
}

void StepperDashboard::showTutorialIntro()
{
    QString message = QString("<b>%1</b><br><br>%2").arg(tutorialTitle, tutorialDescription);

    QMessageBox::information(this, "Tutorial Introduction", message);

    // Luego de mostrar la introducción, mostrar el primer paso
    if (!tutorialSteps.isEmpty()) {
        showStep(0);
    }
}

void StepperDashboard::showStep(int index)
{
    if (index < 0 || index >= tutorialSteps.size()) {
        qDebug() << "Invalid step index: " << index;
        return;
    }

    QJsonObject step = tutorialSteps[index].toObject();
    QString goal = step["goal"].toString();
    QString comment = step["comment"].toString();
    QString help = step["help"].toString();
    QString logAction = step["log"].toString();      // Acción esperada en los logs
    currentReference = step["reference"].toString(); // Guardar la referencia del paso actual
    ui->goalLabel->setText(step["goal"].toString());
    ui->commentButton->setToolTip(step["comment"].toString());
    ui->helpButton->setToolTip(step["help"].toString());

    qDebug() << "Step " << index << " Goal: " << goal;
    qDebug() << "Step " << index << " Log Action: " << logAction;

    // Validar el estado actual del paso
    validateCurrentStep(logAction);

    // Muestra los datos del paso en los widgets correspondientes
    ui->goalLabel->setText(goal);
    ui->commentButton->setToolTip(comment);
    ui->helpButton->setToolTip(help);

    // Validar si el paso ha sido completado
    bool isCompleted = stepValidator->isStepCompleted(logAction.toStdString(), "");
    // Habilitar o deshabilitar el botón "Next"
    ui->nextStepButton->setEnabled(isCompleted);

    //if (index == tutorialSteps.size() - 1) {
    //    ui->nextStepButton->setEnabled(true);
    //    ui->nextStepButton->setText("Finish"); // Cambiar el texto del botón
    //    //connect(ui->nextStepButton, &QPushButton::clicked, this, &StepperDashboard::close);
    //} else {
    //    ui->nextStepButton->setEnabled(isCompleted);
    //    ui->nextStepButton->setText("Next"); // Restaurar el texto del botón
    //}
    if (index == tutorialSteps.size() - 1) {
        ui->nextStepButton->setText("Finish");
    } else {
        ui->nextStepButton->setText("Next");
    }

    if (isCompleted) {
        ui->nextStepButton->setEnabled(true);
        ui->nextStepButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #28a745;" // Fondo verde
            "   color: white;"              // Texto blanco
            "   border: none;"              // Sin bordes
            "   border-radius: 8px;"        // Bordes redondeados
            "   padding: 8px 20px;"         // Espaciado interno
            "   font-size: 16px;"           // Tamaño de fuente
            "   font-weight: bold;"         // Texto en negrita
            "} "
            "QPushButton:hover {"
            "   background-color: #218838;" // Verde más oscuro al pasar el cursor
            "} "
            "QPushButton:pressed {"
            "   background-color: #1e7e34;" // Verde aún más oscuro al presionar
            "}");
    } else {
        ui->nextStepButton->setEnabled(false);

        ui->nextStepButton->setStyleSheet(
            "QPushButton { background-color: #ccc; color: #666; "
            "   border: none;"       // Sin bordes
            "   border-radius: 8px;" // Bordes redondeados
            "   padding: 8px 20px;"  // Espaciado interno
            "   font-size: 16px;"    // Tamaño de fuente
            "   font-weight: bold;"
            "} "
            "QPushButton:hover {"
            "   background-color: #1e7e34;" // Verde aún más oscuro al presionar
            "}");
    }
}

void StepperDashboard::onUserActionPerformed(const std::string &action,
                                             const std::string &componentID)
{
    // Verificar si el paso actual se ha completado
    QJsonObject step = tutorialSteps[currentStepIndex].toObject();
    QString logAction = step["log"].toString();

    if (stepValidator->isStepCompleted(logAction.toStdString(), componentID)) {
        ui->nextStepButton->setEnabled(true);
    }
}

void StepperDashboard::closeEvent(QCloseEvent *event)
{
    if (!codeGenerator->frontendGenerator.isProgressSaved()) {
        if (!showConfirmationDialog(this,
                                    "Unsaved Progress",
                                    "You have unsaved progress. Do you want to exit?")) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void StepperDashboard::validateCurrentStep(const QString &logAction)
{
    if (currentStepIndex < 0 || currentStepIndex >= tutorialSteps.size()) {
        return;
    }
    QJsonObject step = tutorialSteps[currentStepIndex].toObject();
    QString expectedLog = step["log"].toString();

    bool completed = stepValidator->isStepCompleted(expectedLog.toStdString(), "");

    // Habilitar o deshabilitar el botón "Next"
    if (completed) {
        ui->nextStepButton->setEnabled(true);

        ui->nextStepButton->setStyleSheet(
            "QPushButton {"
            "   background-color: #28a745;" // Fondo verde
            "   color: white;"              // Texto blanco
            "   border: none;"              // Sin bordes
            "   border-radius: 8px;"        // Bordes redondeados
            "   padding: 8px 20px;"         // Espaciado interno
            "   font-size: 16px;"           // Tamaño de fuente
            "   font-weight: bold;"         // Texto en negrita
            "} "
            "QPushButton:hover {"
            "   background-color: #218838;" // Verde más oscuro al pasar el cursor
            "} "
            "QPushButton:pressed {"
            "   background-color: #1e7e34;" // Verde aún más oscuro al presionar
            "}");
    } else {
        // Deshabilita el botón hasta que el usuario complete el paso
        ui->nextStepButton->setEnabled(false);

        ui->nextStepButton->setStyleSheet(
            "QPushButton { background-color: #ccc; color: #666; "
            "   border: none;"       // Sin bordes
            "   border-radius: 8px;" // Bordes redondeados
            "   padding: 8px 20px;"  // Espaciado interno
            "   font-size: 16px;"    // Tamaño de fuente
            "   font-weight: bold;"
            "} "
            "QPushButton:hover {"
            "   background-color: #1e7e34;" // Verde aún más oscuro al presionar
            "}");
    }
}

void StepperDashboard::openLastModifiedFile() {
    qDebug() << "Botón presionado: Intentando abrir el último archivo modificado en VS Code";

    QString lastModifiedFile = fileWatcher->getLastModifiedFile();
    std::string projectPath = project.getPath(); // Ruta base del proyecto

    if (!lastModifiedFile.isEmpty()) {
        std::string filePath = lastModifiedFile.toStdString();

        // Validar que el archivo modificado NO sea backend.json o frontend.json
        if (filePath.find("backend.json") != std::string::npos ||
            filePath.find("frontend.json") != std::string::npos) {
            qDebug() << "⚠ Se modificó un archivo de configuración JSON. Buscando archivos dentro de frontend/ o backend/";

            // Buscar el archivo más reciente dentro de frontend/
            std::string frontendFile = fileWatcher->getLastModifiedFileInFolder(projectPath + "/frontend/src");
            if (!frontendFile.empty()) {
                qDebug() << "📂 Último archivo modificado en Frontend: " << QString::fromStdString(frontendFile);
                FileOpener::openInVSCode(projectPath + "/frontend/src", frontendFile);
                return;
            }

            // Buscar el archivo más reciente dentro de backend/
            std::string backendFile = fileWatcher->getLastModifiedFileInFolder(projectPath + "/backend/src");
            if (!backendFile.empty()) {
                qDebug() << "📂 Último archivo modificado en Backend: " << QString::fromStdString(backendFile);
                FileOpener::openInVSCode(projectPath + "/backend/src", backendFile);
                return;
            }

            qDebug() << "⚠ No se encontraron archivos recientes en frontend/ o backend/. Abriendo carpeta completa.";
            FileOpener::openInVSCode(projectPath);
        } else {
            qDebug() << "Último archivo modificado: " << QString::fromStdString(filePath);

            // Determinar si pertenece a frontend/ o backend/
            if (filePath.find("/frontend/src") != std::string::npos) {
                qDebug() << "📂 Última modificación en Frontend.";
                FileOpener::openInVSCode(projectPath + "/frontend/src", filePath);
            } else if (filePath.find("/backend/src") != std::string::npos) {
                qDebug() << "📂 Última modificación en Backend.";
                FileOpener::openInVSCode(projectPath + "/backend/src", filePath);
            } else {
                qDebug() << "⚠ Archivo fuera de frontend/ y backend/. Abriendo proyecto completo.";
                FileOpener::openInVSCode(projectPath);
            }
        }
    } else {
        qDebug() << "⚠ No hay archivos modificados recientemente. Abriendo proyecto completo en VS Code.";
        FileOpener::openInVSCode(projectPath);
    }
}
void StepperDashboard::setupFloatingButton() {
    // Crear el botón flotante
    floatingButton = new QPushButton(this);
    floatingButton->setIcon(QIcon(":/icons/VS.png")); // Ruta del icono
    floatingButton->setIconSize(QSize(36, 36));       // Ajustar tamaño del icono
    floatingButton->setFixedSize(65, 65);             // Hace que el botón sea circular
    floatingButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #E1E1E1;"                  // Azul de VS Code
        "   border-radius: 32px;"                        // Lo hace circular
        "   border: 1px solid #0F66DE;"                  // Borde azul de VS Code
        "   box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);" // Efecto flotante
        "}"
        "QPushButton:hover {"
        "   background-color: #0F66DE;"            // Color más oscuro al pasar el mouse
        "   border: 1px solid #0F66DE;"            // Borde azul de VS Code
        "   qproperty-icon: url(:/icons/vs2.png);" // Icono en hover
        "}"
        "QPushButton:pressed {"
        "   background-color: #003F73;" // Color más oscuro al presionar
        "   border: 1px solid #003F73;" // Borde azul de VS Code
        "}");

    // Conectar el botón a la función de abrir VS Code
    connect(floatingButton, &QPushButton::clicked, this, &StepperDashboard::openLastModifiedFile);

    // Crear botones desplegables (inicialmente ocultos)
    backendButton = new QPushButton(this);
    frontendButton = new QPushButton(this);
    lastFileButton = new QPushButton(this);


    QList<QPushButton*> buttons = {backendButton, frontendButton, lastFileButton};
    for (QPushButton* btn : buttons) {
        btn->setFixedSize(50, 50);
        btn->setVisible(false);
        btn->setStyleSheet("QPushButton {"
                           "   background-color: #E1E1E1;"
                           "   border-radius: 25px;"
                           "   border: 1px solid #0F66DE;" // Borde azul de VS Code
                           "   color: white;"
                           "   font-size: 14px;"
                           "}"
                           "QPushButton:hover {"
                           "   background-color: #0F66DE;"
                           "}");
    }

    backendButton->setIcon(QIcon(":/icons/exp.png")); // 🔼 Botón para abrir `backend`
    frontendButton->setIcon(QIcon(":/icons/Icono React.webp")); // 🔼 Botón para abrir `frontend`
    lastFileButton->setIcon(QIcon(":/icons/recent2.png"));// ◀ Botón para abrir último archivo

    // Conectar botones a funciones
    connect(backendButton, &QPushButton::clicked, this, &StepperDashboard::openBackendInVSCode);
    connect(frontendButton, &QPushButton::clicked, this, &StepperDashboard::openFrontendInVSCode);
    connect(lastFileButton, &QPushButton::clicked, this, &StepperDashboard::openLastModifiedFile);

    // Posicionar el botón sobre todo el contenido (sin afectar layouts)
    floatingButton->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    floatingButton->raise();                           // Asegura que esté sobre todo
    floatingButton->move(width() - 95, height() - 95); // Ajusta su posición
    // Inicializar animaciones
    createAnimations();
}

// Muestra/Oculta los botones cuando se hace clic derecho
void StepperDashboard::contextMenuEvent(QContextMenuEvent *event) {
    Q_UNUSED(event);
    toggleExtraButtons();
}

// Función para mostrar/ocultar botones
void StepperDashboard::toggleExtraButtons() {
    bool isVisible = backendButton->isVisible();

    backendButton->setVisible(!isVisible);
    frontendButton->setVisible(!isVisible);
    lastFileButton->setVisible(!isVisible);

    if (!isVisible) {
        backendAnimation->setStartValue(floatingButton->pos());
        backendAnimation->setEndValue(QPoint(floatingButton->x(), floatingButton->y() - 67));
        backendAnimation->start();

        frontendAnimation->setStartValue(floatingButton->pos());
        frontendAnimation->setEndValue(QPoint(floatingButton->x(), floatingButton->y() - 130));
        frontendAnimation->start();

        lastFileAnimation->setStartValue(floatingButton->pos());
        lastFileAnimation->setEndValue(QPoint(floatingButton->x() - 67, floatingButton->y()));
        lastFileAnimation->start();
    } else {
        backendAnimation->setStartValue(backendButton->pos());
        backendAnimation->setEndValue(floatingButton->pos());
        backendAnimation->start();

        frontendAnimation->setStartValue(frontendButton->pos());
        frontendAnimation->setEndValue(floatingButton->pos());
        frontendAnimation->start();

        lastFileAnimation->setStartValue(lastFileButton->pos());
        lastFileAnimation->setEndValue(floatingButton->pos());
        lastFileAnimation->start();

        // Retrasar la ocultación de los botones
        QTimer::singleShot(300, this, [=]() {
            backendButton->setVisible(false);
            frontendButton->setVisible(false);
            lastFileButton->setVisible(false);
        });
    }
}

// Abrir la carpeta `backend` en VS Code
void StepperDashboard::openBackendInVSCode() {
    std::string backendPath = project.getPath() + "/backend";
    qDebug() << "📂 Abriendo Backend en VS Code: " << QString::fromStdString(backendPath);

    if (!FileOpener::openInVSCode(backendPath)) {
        qDebug() << "❌ Error al abrir VS Code en la carpeta Backend.";
    }
}

// Abrir la carpeta `frontend` en VS Code
void StepperDashboard::openFrontendInVSCode() {
    std::string frontendPath = project.getPath() + "/frontend";
    qDebug() << "📂 Abriendo Frontend en VS Code: " << QString::fromStdString(frontendPath);

    if (!FileOpener::openInVSCode(frontendPath)) {
        qDebug() << "❌ Error al abrir VS Code en la carpeta Frontend.";
    }
}

// Sobrescribir `resizeEvent()` para mantener la posición del botón flotante
void StepperDashboard::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event); // Llamar a la implementación base

    // Mover el botón flotante a la esquina inferior derecha
    floatingButton->move(width() - 90, height() - 90);

    // Si los botones extras están visibles, los reubica correctamente
    if (backendButton->isVisible()) {
        backendButton->move(floatingButton->x(), floatingButton->y() - 60);
        frontendButton->move(floatingButton->x(), floatingButton->y() - 120);
        lastFileButton->move(floatingButton->x() - 60, floatingButton->y());
    }
}

void StepperDashboard::createAnimations() {
    backendAnimation = new QPropertyAnimation(backendButton, "pos");
    frontendAnimation = new QPropertyAnimation(frontendButton, "pos");
    lastFileAnimation = new QPropertyAnimation(lastFileButton, "pos");

    backendAnimation->setDuration(300);
    frontendAnimation->setDuration(300);
    lastFileAnimation->setDuration(300);

    backendAnimation->setEasingCurve(QEasingCurve::OutBack);
    frontendAnimation->setEasingCurve(QEasingCurve::OutBack);
    lastFileAnimation->setEasingCurve(QEasingCurve::OutBack);
}
