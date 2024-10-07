#ifndef PROJECTSPANEL_H
#define PROJECTSPANEL_H

#include <QWidget>
#include "../../models/project/project.h"
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

private:
    Ui::ProjectsPanel *ui;
    std::vector<Project> projects;
};

#endif // PROJECTSPANEL_H
