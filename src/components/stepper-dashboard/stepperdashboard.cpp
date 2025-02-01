#include "stepperdashboard.h"
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

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

nlohmann::json tutorialData;

StepperDashboard::StepperDashboard(QDialog *parent, const Project &project)
    : QDialog(parent)
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
{
    ui->setupUi(this);

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

    // Cargar los datos del tutorial desde el archivo JSON
    loadTutorialData();

    // Mostrar el primer paso del primer tutorial
    showStep(0);

    // Conectar botones de la barra de tutoriales
    setupTutorialConnections();
    // Muestra la barra de tutoriales solo si la opción de tutoriales está activa
    initializeTutorialBar(project.isTutorialEnabled());
    setupTutorialConnections();
}

void StepperDashboard::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    QTimer::singleShot(0, this, [this]() {
        // Backend
        if (codeGenerator->backendGenerator.loadSchema()) {
            QMessageBox::information(this, "Successful", "Information loaded");

            emit backendSchemaLoaded();
        } else {
            QMessageBox::warning(this, "Warning", "There is no information, add data");
        }
        // Frontend
        if (codeGenerator->frontendGenerator.loadSchema()) {
            QMessageBox::information(this, "Successful", "Views loaded");

            emit frontendSchemaLoaded();
        } else {
            QMessageBox::warning(this, "Warning", "There is no views, add visual content");
        }
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
    auto views = codeGenerator->frontendGenerator.getViews();
    auto custComponents = codeGenerator->frontendGenerator
                                                                .getCustomComponents();
    std::vector<Route> routes = codeGenerator->frontendGenerator.getRoutes();

    frontendDashboard->setViews(views);
    frontendDashboard->setRoutes(routes);
    frontendDashboard->setCustomComponents(custComponents);

    frontendDashboard->fillAvailableSections();

    if (!views.empty()) {
        frontendDashboard->setCurrentSection(views.at(0));
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

void StepperDashboard::onSaveChanges()
{
    codeGenerator->backendGenerator.setTransactions(backendDashboard->getTransactions());

    codeGenerator->backendGenerator.updateBackendCode();

    codeGenerator->frontendGenerator.setRoutes(frontendDashboard->getRoutes());

    codeGenerator->frontendGenerator.setViews(frontendDashboard->getViews());

    codeGenerator->frontendGenerator.setCustomComponents(frontendDashboard->getCustomComponents());

    if (codeGenerator->frontendGenerator.updateFrontendCode()) {
        QMessageBox::information(this, "Save Changes", "Changes have been saved successfully.");
    } else {
        QMessageBox::warning(this, "Failed", "Failed to update JSON and generate code.");
    }
}

void StepperDashboard::onCreateVersion()
{
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
    // Cerrar el StepperDashboard
    this->close();

    // Crear y mostrar el ProjectsPanel
    Stepper *createProjects = new Stepper();
    createProjects->setAttribute(Qt::WA_DeleteOnClose); // Liberar memoria automáticamente al cerrar
    createProjects->show();
}
void StepperDashboard::initializeTutorialBar(bool showTutorials)
{
    // Muestra u oculta la barra de tutoriales según el condicional
    //foreach (QWidget *widget, tutorialBar->findChildren<QWidget *>()) {
    //    widget->setVisible(false); // Cambia a true para mostrarlo.
    //}
}

void StepperDashboard::setupTutorialConnections()
{
    connect(ui->nextStepButton,
            &QPushButton::clicked,
            this,
            &StepperDashboard::goToNextTutorialStep);
}

void StepperDashboard::showTutorialComment()
{
    QString comment = ui->commentButton->toolTip(); // Obtener el tooltip asignado en showStep()

    if (!comment.isEmpty()) {
        QMessageBox::information(this, "Comment", comment);
    } else {
        QMessageBox::warning(this, "Comment", "No comment available.");
    }
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
    if (currentStepIndex + 1 < tutorialSteps.size()) {
        currentStepIndex++;
        showStep(currentStepIndex); // Pasa el índice actual a la función
    } else {
        QMessageBox::information(this, "Tutorial", "You have completed all steps.");
    }
}

void StepperDashboard::loadTutorialData()
{
    QFile tutorialFile(":/resources/log_tutorials/begginer.json");
    if (!tutorialFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open the tutorial JSON file.");
        return;
    }

    QByteArray data = tutorialFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isNull()) {
        QMessageBox::critical(this, "Error", "Invalid JSON format.");
        tutorialFile.close();
        return;
    }

    tutorialFile.close();

    // Obtén el array de pasos del primer tutorial
    QJsonArray tutorials = jsonDoc.array();
    if (!tutorials.isEmpty()) {
        QJsonObject firstTutorial = tutorials.at(0).toObject(); // Primer tutorial
        tutorialSteps = firstTutorial["steps"].toArray();
    }

    // Muestra el primer paso
    if (!tutorialSteps.isEmpty()) {
        showStep(0);
    }
}

void StepperDashboard::showStep(int index)
{
    if (index < 0 || index >= tutorialSteps.size()) {
        return; // Verifica que el índice esté dentro de los límites
    }

    QJsonObject step = tutorialSteps[index].toObject();
    QString goal = step["goal"].toString();
    QString comment = step["comment"].toString();
    QString help = step["help"].toString();

    // Muestra los datos del paso en los widgets correspondientes
    ui->goalLabel->setText(goal);
    ui->commentButton->setToolTip(comment);
    ui->helpButton->setToolTip(help);
}
