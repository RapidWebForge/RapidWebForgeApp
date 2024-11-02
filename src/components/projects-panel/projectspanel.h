#ifndef PROJECTSPANEL_H
#define PROJECTSPANEL_H

#include <QWidget>
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
    void onAddProjectClicked();
    void onProjectPreviewClicked(const Project &project);

    void on_configurationButton_clicked();

private:
    Ui::ProjectsPanel *ui;
    std::vector<Project> projects;
    ConfigurationView *configView = nullptr;
    bool checkCommand(const std::string &command, bool dobleQuote = true);
    void applyStylesProj();
    ConfigurationManager *confManager = nullptr;
};

#endif // PROJECTSPANEL_H
