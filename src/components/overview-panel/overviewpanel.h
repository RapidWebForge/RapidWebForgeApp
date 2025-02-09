#ifndef OVERVIEWPANEL_H
#define OVERVIEWPANEL_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include "../../core/configuration-manager/configurationmanager.h"
#include "../../models/project/project.h"
#include "../stepper/stepper.h"
#include <vector>

namespace Ui {
class OverviewPanel;
}

class OverviewPanel : public QDialog
{
    Q_OBJECT

public:
    explicit OverviewPanel(QWidget *parent = nullptr);
    ~OverviewPanel();

    void setupProjects(const std::vector<Project> &projects); // 🚀 Agregamos esta función

private:
    Ui::OverviewPanel *ui;
    QGridLayout *gridLayout;
    std::vector<Project> projects;
    ConfigurationManager *confManager = nullptr;

private slots:
    void onAddProjectClicked();
    void onProjectPreviewClicked(const Project &project);
    void onDeleteProjectRequested(int projectId);

signals:
    void projectCreationRequested();
    void projectClicked(const Project &project);
    void deleteRequested(int projectId);
    void openTutorial(const QString &tutorialPath);
};

#endif // OVERVIEWPANEL_H
