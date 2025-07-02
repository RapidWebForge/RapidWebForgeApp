#ifndef EDITPROJECT_H
#define EDITPROJECT_H

#include <QDialog>
#include "../../models/project/project.h"

namespace Ui {
class EditProject;
}

class EditProject : public QDialog
{
    Q_OBJECT

public:
    explicit EditProject(Project &project, QWidget *parent = nullptr);
    ~EditProject();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::EditProject *ui;
    Project &project;
};

#endif // EDITPROJECT_H
