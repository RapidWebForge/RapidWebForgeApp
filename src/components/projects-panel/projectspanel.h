#ifndef PROJECTSPANEL_H
#define PROJECTSPANEL_H

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "../../../src/components/overview-panel/overviewpanel.h"
#include "../../../src/components/pro-panel/propanel.h"
#include "../../../src/components/templates-panel/templatespanel.h"
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

private:
    Ui::ProjectsPanel *ui;
    OverviewPanel *overviewPanel;
    TutorialsPanel *tutorialsPanel;
    ProPanel *proPanel;
    TemplatesPanel *templatesPanel;
    QWidget *recentsPage;
    QWidget *tutorialsPage;
    QWidget *projectsPage;
    ProjectManager projectManager;
    ConfigurationView *configView = nullptr;
    void applyStylesProj();
    ConfigurationManager *confManager = nullptr;
    QStackedWidget *stackedWidget;

private slots:
    void on_configurationButton_clicked();
    void on_recentsButton_clicked();
    void on_tutorialButton_clicked();
    void on_projectButton_clicked();
    void on_templatesButton_clicked();
};

#endif // PROJECTSPANEL_H
