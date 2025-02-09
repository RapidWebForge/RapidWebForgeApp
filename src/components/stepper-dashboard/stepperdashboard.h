#ifndef STEPPERDASHBOARD_H
#define STEPPERDASHBOARD_H

#include <QAction>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QString>
#include <QWidget>
#include "../../core/code-generator/codegenerator.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/version-manager/versionmanager.h"
#include "../../models/project/project.h"
#include "../backend-dashboard/backenddashboard.h"
#include "../frontend-dashboard/frontenddashboard.h"
#include <nlohmann/json.hpp>
#include <variant> // 📌 Incluir std::variant

namespace Ui {
class StepperDashboard;
}

class StepperDashboard : public QDialog
{
    Q_OBJECT

public:
    // 📌 Constructor para proyectos normales
    explicit StepperDashboard(QDialog *parent = nullptr,
                              const Project &project = Project(),
                              const QString &tutorialPath = "");
    ~StepperDashboard();

    void loadTutorialData(); // 📌 Ahora `loadData()` maneja proyectos y tutoriales en un solo método

protected:
    void showEvent(QShowEvent *event) override;

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
    void showTutorialComment();  // Muestra el comentario del tutorial
    void showTutorialHelp();     // Muestra la ayuda del tutorial
    void goToNextTutorialStep(); // Avanza al siguiente paso del tutorial
    void showTutorialIntro();

private:
    Ui::StepperDashboard *ui;
    BackendDashboard *backendDashboard;
    FrontendDashboard *frontendDashboard;

    // Tutorial bar methods
    void initializeTutorialBar();    // Método para inicializar la barra de tutoriales
    void setupTutorialConnections(); // Conecta los botones de tutorial a sus funciones
    // Definición de menús
    QMenu *projectMenu;
    QMenu *versionsMenu;

    // 📌 Variable unificada para manejar tutoriales o proyectos
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

    // 📌 Nuevo parámetro para almacenar la ruta del tutorial JSON
    QString tutorialPath;
    QString tutorialFilePath; // Ruta del tutorial JSON

    QJsonArray tutorialSteps; // Array para almacenar los pasos del tutorial

    void showStep(int index); // Función para mostrar un paso

    QString tutorialTitle;
    QString tutorialDescription;
    QString currentReference;
};

#endif // STEPPERDASHBOARD_H
