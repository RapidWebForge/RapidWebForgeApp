#ifndef STEPPERDASHBOARD_H
#define STEPPERDASHBOARD_H

#include <QAction>
#include <QMenu>
#include <QWidget>
#include "../../core/code-generator/codegenerator.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/version-manager/versionmanager.h"
#include "../../models/project/project.h"
#include "../backend-dashboard/backenddashboard.h"
#include "../frontend-dashboard/frontenddashboard.h"

namespace Ui {
class StepperDashboard;
}

class StepperDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit StepperDashboard(QDialog *parent = nullptr, const Project &project = Project());
    ~StepperDashboard();

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

private:
    Ui::StepperDashboard *ui;
    BackendDashboard *backendDashboard;
    FrontendDashboard *frontendDashboard;
    // Definición de menús
    QMenu *projectMenu;
    QMenu *versionsMenu;

    // Definición de acciones para los menús
    QAction *projectChangeAction;
    QAction *createNewProjectAction;
    QAction *saveChangesAction;
    QAction *deleteProjectAction;
    QAction *deployProjectAction;

    QAction *createVersionAction;
    QAction *changeVersionAction;
    QAction *versionHistoryAction;
    QAction *deleteVersionAction;

    // Code Generator definition
    CodeGenerator *codeGenerator;

    // Version Manager
    VersionManager *versionManager;

    // Project
    Project project;
};

#endif // STEPPERDASHBOARD_H
