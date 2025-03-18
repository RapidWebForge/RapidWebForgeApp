#ifndef STEPPERDASHBOARD_H
#define STEPPERDASHBOARD_H

#include <QAction>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "../../core/code-generator/codegenerator.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/logging/stepvalidator.h"
#include "../../core/version-manager/versionmanager.h"
#include "../../models/project/project.h"
#include "../backend-dashboard/backenddashboard.h"
#include "../frontend-dashboard/frontenddashboard.h"
#include <nlohmann/json.hpp>
#include <variant>

namespace Ui {
class StepperDashboard;
}

class StepperDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit StepperDashboard(QWidget *parent = nullptr,
                              const Project &project = Project(),
                              const QString &tutorialPath = "");
    ~StepperDashboard();

    void loadTutorialData();
    bool showConfirmationDialog(QWidget *parent, const QString &title, const QString &message);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void backendSchemaLoaded();
    void frontendSchemaLoaded();
    void projectDeleteRequested(const Project &project);

private slots:
    void showBackendPage();
    void showFrontendPage();
    void applyMenuStyles();
    void setupMenus();
    void onBackendSchemaLoaded();
    void onFrontendSchemaLoaded();
    void onSaveChanges();
    void onCreateVersion();
    void onChangeVersion();
    void onVersionHistory();
    void onDeleteVersion();
    void onDeployProject();
    void onProjectChange();
    void onCreateProject();
    // Slots relacionados con los tutoriales
    void showTutorialComment();
    void showTutorialHelp();
    void goToNextTutorialStep();
    void showTutorialIntro();
    void onUserActionPerformed(const std::string &action, const std::string &componentID);

private:
    Ui::StepperDashboard *ui;
    BackendDashboard *backendDashboard;
    FrontendDashboard *frontendDashboard;
    StepValidator *stepValidator;

    // Tutorial bar methods
    void initializeTutorialBar();
    void setupTutorialConnections();
    // Definición de menús
    QMenu *projectMenu;
    QMenu *versionsMenu;

    std::variant<Project, QString> dataVariant;
    bool isTutorialMode = false;

    // Definición de acciones para los menús
    QAction *projectChangeAction;
    QAction *createNewProjectAction;
    QAction *saveChangesAction;
    QAction *deployProjectAction;

    QAction *createVersionAction;
    QAction *changeVersionAction;
    QAction *versionHistoryAction;
    QAction *deleteVersionAction;

    // Variables para manejar el tutorial
    nlohmann::json tutorialData;  // Almacena los datos del JSON
    int currentTutorialIndex = 0; // Índice del tutorial actual

    // Métodos privados
    void showTutorialStep(int tutorialIndex, int stepIndex); // Mostrar el paso actual

    // Code Generator definition
    CodeGenerator *codeGenerator;

    // Version Manager
    VersionManager *versionManager;

    // Project
    Project project;
    int currentStepIndex = 0; // Inicializa el índice en 0

    QString tutorialPath;
    QString tutorialFilePath;

    QJsonArray tutorialSteps;

    void showStep(int index);
    QTimer *stepCheckTimer;
    QString tutorialTitle;
    QString tutorialDescription;
    QString currentReference;
};

#endif // STEPPERDASHBOARD_H
