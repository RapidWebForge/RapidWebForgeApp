#ifndef PROJECTSPANEL_H
#define PROJECTSPANEL_H

#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include "../../../src/components/overview-panel/overviewpanel.h"
#include "../../../src/components/pro-panel/propanel.h"
#include "../../../src/components/tutorials-panel/tutorialspanel.h"
#include "../../core/configuration-manager/configurationmanager.h"
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
    void setupProjects(const std::vector<Project> &projects);

private slots:
    void showRecents();
    void showTutorials();
    void showProjects();
    void onAddProjectClicked();
    void onProjectPreviewClicked(const Project &project);
    void onDeleteProjectRequested(int projectId);
    void on_configurationButton_clicked();

private:
    Ui::ProjectsPanel *ui;
    OverviewPanel *overviewPanel;
    TutorialsPanel *tutorialsPanel;
    ProPanel *proPanel;
    QWidget *recentsPage;
    QWidget *tutorialsPage;
    QWidget *projectsPage;
    std::vector<Project> projects;
    ConfigurationView *configView = nullptr;
    bool checkCommand(const std::string &command, bool dobleQuote = true);
    void applyStylesProj();
    ConfigurationManager *confManager = nullptr;
    QStackedWidget *stackedWidget;
};

#endif // PROJECTSPANEL_H
