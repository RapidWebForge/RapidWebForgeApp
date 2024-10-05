#ifndef STEPPER_H
#define STEPPER_H

#include <QWidget>
#include "../../core/project-manager/projectmanager.h"
#include "../../models/project/project.h"
#include "../backend-assistant/backendassistant.h"
#include "../creation-assistant/creationassistant.h"
#include "../database-assistant/databaseassistant.h"
#include "../frontend-assistant/frontendassistant.h"
#include "../summary-assistant/summaryassistant.h"

namespace Ui {
class Stepper;
}

class Stepper : public QWidget
{
    Q_OBJECT

public:
    explicit Stepper(QWidget *parent = nullptr);
    ~Stepper();

private slots:
    void on_backButton_clicked();

    void on_nextButton_clicked();

private:
    Ui::Stepper *ui;
    void applyStyles(); // Declaración de la función applyStyles
    CreationAssistant *creationAssistant;
    DatabaseAssistant *databaseAssistant;
    FrontendAssistant *frontendAssistant;
    BackendAssistant *backendAssistant;
    SummaryAssistant *summaryAssistant;
    Project newProject;
    ProjectManager projectManager;
};

#endif // STEPPER_H
