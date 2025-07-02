#include "editproject.h"
#include <QMessageBox>
#include <QTimer>
#include "../../core/project-manager/projectmanager.h"
#include "ui_editproject.h"
#include <qregularexpression.h>

EditProject::EditProject(Project &project, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditProject)
    , project(project)
{
    ui->setupUi(this);

    ui->projectNameLineEdit->setText(QString::fromStdString(project.getName()));
    ui->descriptionPlainTextEdit->setPlainText(QString::fromStdString(project.getDescription()));
}

EditProject::~EditProject()
{
    delete ui;
}

void EditProject::on_buttonBox_accepted()
{
    ProjectManager projectManager;

    // QString name = ui->projectNameLineEdit->text();
    QString description = ui->descriptionPlainTextEdit->toPlainText();
    QRegularExpression invalidChars(R"([\\/:*?"<>|])");
    if (invalidChars.match(description).hasMatch()) {
        QMessageBox::warning(this, "Error", "Your description have invalid characters.");
        return;
    }

    // if (name.isEmpty()) {
    //     QMessageBox::warning(this, "Error", "Project name cannot be empty.");
    //     return;
    // }

    // project.setName(name.toStdString());
    project.setDescription(description.toStdString());

    projectManager.updateProject(project);
}
