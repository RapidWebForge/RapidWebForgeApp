#ifndef STEPPERDASHBOARD_H
#define STEPPERDASHBOARD_H

#include <QAction>
#include <QMenu>
#include <QWidget>
#include "../../core/code-generator/codegenerator.h"
#include "../backend-dashboard/backenddashboard.h"
#include "../frontend-dashboard/frontenddashboard.h"

namespace Ui {
class StepperDashboard;
}

class StepperDashboard : public QDialog
{
    Q_OBJECT

public:
    explicit StepperDashboard(QDialog *parent = nullptr);
    ~StepperDashboard();

protected:
    void showEvent(QShowEvent *event) override;

signals:
    void schemaLoaded();

private slots:
    void showBackendPage();
    void showFrontendPage();
    void applyMenuStyles();
    void setupMenus(); // Método para inicializar y configurar los menús
    std::string defineProjectPath();
    void onSchemaLoaded();

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

    QAction *createVersionAction;
    QAction *changeVersionAction;
    QAction *versionHistoryAction;
    QAction *deleteVersionAction;

    // Code Generator definition
    CodeGenerator *codeGenerator;
};

#endif // STEPPERDASHBOARD_H
