#ifndef PROJECTSPANEL_H
#define PROJECTSPANEL_H

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "../../../src/components/overview-panel/overviewpanel.h"
#include "../../../src/components/pro-panel/propanel.h"
#include "../../../src/components/tutorials-panel/tutorialspanel.h"
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../core/project-manager/projectmanager.h"
#include "../../models/project/project.h"
#include "../configuration-view/configurationview.h"
#include <vector>

namespace Ui {
class ProjectsPanel;
}

class ProjectsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectsPanel(QWidget *parent = nullptr);
    ~ProjectsPanel();

private slots:
    void showRecents();
    void showTutorials();
    void showProjects();
    void on_configurationButton_clicked();

private:
    Ui::ProjectsPanel *ui;
    OverviewPanel *overviewPanel;
    TutorialsPanel *tutorialsPanel;
    ProPanel *proPanel;
    QWidget *recentsPage;
    QWidget *tutorialsPage;
    QWidget *projectsPage;
    ProjectManager projectManager;
    ConfigurationView *configView = nullptr;
    void applyStylesProj();
    ConfigurationManager *confManager = nullptr;
    QStackedWidget *stackedWidget;
};

#endif // PROJECTSPANEL_H
