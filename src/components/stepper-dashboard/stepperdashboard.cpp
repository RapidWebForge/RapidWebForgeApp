#include "stepperdashboard.h"
#include <QMessageBox>
#include <QTimer>
#include "../../components/create-version/createversion.h"
#include "../../components/manage-version/manageversion.h"
#include "../../components/version-history/versionhistory.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/deploy-manager/deploymanager.h"
#include "ui_stepperdashboard.h"
#include <fmt/core.h>

StepperDashboard::StepperDashboard(QDialog *parent, const Project &project)
    : QDialog(parent)
    , ui(new Ui::StepperDashboard)
    , frontendDashboard(new FrontendDashboard())
    , backendDashboard(new BackendDashboard())
    , projectMenu(new QMenu("Project", this))
    , versionsMenu(new QMenu("Versions", this))
    , projectChangeAction(new QAction("Project change", this))
    , createNewProjectAction(new QAction("Create new project", this))
    , saveChangesAction(new QAction("Save changes", this))
    , deleteProjectAction(new QAction("Delete project", this))
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

    // Conectar la señal de BackendDashboard para que se guarden los cambios
    connect(backendDashboard,
            &BackendDashboard::transactionNameChanged,
            this,
            &StepperDashboard::onSaveChanges);

    // Conectar los botones a los slots
    connect(ui->backendButton, &QPushButton::clicked, this, &StepperDashboard::showBackendPage);
    connect(ui->frontendButton, &QPushButton::clicked, this, &StepperDashboard::showFrontendPage);

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
    std::vector<View> views = codeGenerator->frontendGenerator.getViews();
    std::vector<Route> routes = codeGenerator->frontendGenerator.getRoutes();

    frontendDashboard->setViews(views);
    frontendDashboard->setRoutes(routes);

    if (!views.empty()) {
        frontendDashboard->setCurrentView(views.at(0));
    }
}

StepperDashboard::~StepperDashboard()
{
    delete ui;
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
}

void StepperDashboard::setupMenus()
{
    // Configurar acciones para el menú de Project
    projectMenu->addAction(projectChangeAction);
    projectMenu->addAction(createNewProjectAction);
    projectMenu->addAction(saveChangesAction);
    projectMenu->addSeparator(); // Añadir un separador
    projectMenu->addAction(deleteProjectAction);
    projectMenu->addAction(deployProjectAction);

    // Configurar las acciones de cada opción
    deleteProjectAction->setEnabled(false); // Deshabilitar la opción de eliminar para estilo

    // Configurar acciones para el menú de Versions
    versionsMenu->addAction(createVersionAction);
    versionsMenu->addAction(changeVersionAction);
    versionsMenu->addAction(versionHistoryAction);
    versionsMenu->addSeparator(); // Añadir un separador
    versionsMenu->addAction(deleteVersionAction);

    // Configurar las acciones de cada opción
    // deleteVersionAction->setEnabled(false); // Deshabilitar la opción de eliminar para estilo

    // Conectar señales de las acciones a slots si es necesario
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

    if (codeGenerator->frontendGenerator.updateFrontendCode()) {
        QMessageBox::information(this, "Successful", "View and JSON updated");
    } else {
        QMessageBox::warning(this, "Failed", "Failed to update JSON and generate code.");
    }

    QMessageBox::information(this, "Save Changes", "Changes have been saved successfully.");
}

void StepperDashboard::onCreateVersion()
{
    CreateVersion dialog(this);

    dialog.exec();

    // TODO: Usar el version manager
}

void StepperDashboard::onChangeVersion()
{
    // TODO: Validar el Manage Version para "Cambiar"
    ManageVersion dialog(this);

    dialog.exec();

    // TODO: Usar el version manager
}

void StepperDashboard::onDeleteVersion()
{
    // TODO: Validar el Manage Version para "Eliminar"
    ManageVersion dialog(this);

    dialog.exec();

    // TODO: Usar el version manager
}

void StepperDashboard::onVersionHistory()
{
    VersionHistory dialog(this);

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
        DeployManager deployManager(codeGenerator->getProjectPath(), ngInxPath);
        deployManager.start(bunPath);
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Critical Error", e.what());
        return;
    }
}
