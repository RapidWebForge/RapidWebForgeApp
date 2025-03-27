#ifndef PROPANEL_H
#define PROPANEL_H

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
class ProPanel;
}

class ProPanel : public QDialog
{
    Q_OBJECT

public:
    explicit ProPanel(QWidget *parent = nullptr);
    ~ProPanel();

    void setupProjects(const std::vector<Project> &projects); // 🚀 Agregamos esta función

private:
    Ui::ProPanel *ui;
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
};

#endif // PROPANEL_H
