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
#include "../custom-tree-widget/customtreewidget.h"
#include "../frontend-dashboard/frontenddashboard.h"
#include <nlohmann/json.hpp>
#include <variant> // 📌 Incluir std::variant
#include <QResizeEvent>
#include <QPushButton> // 📌 Importar QPushButton
#include <QEnterEvent>  // 📌 Importar QEnterEvent
#include <QPropertyAnimation>  // ✅ Para animaciones

#include <QPushButton>  // Agregar botón
#include "../../utils/file/FileWatcher.h"  // Detectar archivos modificados
#include "../../utils/vscode/FileOpener.h" // Abrir VS Code

namespace Ui {
class StepperDashboard;
}

class StepperDashboard : public QWidget
{
    Q_OBJECT

public:
    // 📌 Constructor para proyectos normales
    explicit StepperDashboard(QWidget *parent = nullptr,
                              const Project &project = Project(),
                              const QString &tutorialPath = "");
    ~StepperDashboard();

    void loadTutorialData(); // 📌 Ahora `loadData()` maneja proyectos y tutoriales en un solo método
public slots:
    void validateCurrentStep(const QString &logAction);

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override; // 📌 Sobreescribir resizeEvent
    void contextMenuEvent(QContextMenuEvent *event) override;  // 📌 Detectar clic derecho

signals:
    void backendSchemaLoaded();
    void frontendSchemaLoaded();
    void projectDeleteRequested(const Project &project);
    void stepUpdated(const QString &logAction);

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
    void onUserActionPerformed(const std::string &action, const std::string &componentID);


private:
    Ui::StepperDashboard *ui;
    BackendDashboard *backendDashboard;
    FrontendDashboard *frontendDashboard;
    StepValidator *stepValidator;
    CustomTreeWidget *customTreeWidget; // 🆕 Se declara un puntero a CustomTreeWidget
    QPushButton *floatingButton;  // 📌 Declarar el botón flotante
    QPushButton *backendButton;   // 📌 Botón para abrir backend
    QPushButton *frontendButton;  // 📌 Botón para abrir frontend
    QPushButton *lastFileButton;  // 📌 Botón para abrir el último archivo modificado

    QPropertyAnimation *backendAnimation;   // ✅ Animación para backend
    QPropertyAnimation *frontendAnimation;  // ✅ Animación para frontend
    QPropertyAnimation *lastFileAnimation;  // ✅ Animación para último archivo

    // Tutorial bar methods
    void initializeTutorialBar();    // Método para inicializar la barra de tutoriales
    void setupTutorialConnections(); // Conecta los botones de tutorial a sus funciones
    // 📌 Nueva función para el botón flotante
    void setupFloatingButton();
    void toggleExtraButtons();    // 📌 Mostrar/Ocultar botones desplegables
    void createAnimations();

    void openBackendInVSCode();
    void openFrontendInVSCode();
    void openLastModifiedFile();

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
    QTimer *stepCheckTimer;
    QString tutorialTitle;
    QString tutorialDescription;
    QString currentReference;

    QPushButton *btnOpenVSCode;  // 📌 Botón para abrir VS Code
    FileWatcher *fileWatcher;    // 📌 Instancia para monitorear archivos
};

#endif // STEPPERDASHBOARD_H
