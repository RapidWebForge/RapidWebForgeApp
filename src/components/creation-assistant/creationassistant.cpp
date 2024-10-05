#include "creationassistant.h"
#include "ui_creationassistant.h"
#include <QFileDialog>
#include <QMessageBox>

CreationAssistant::CreationAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreationAssistant)
{
    ui->setupUi(this);

    // TODO: Pass to a function
    ui->projectInfoGroupBox->setStyleSheet("border: none;");

    ui->projectNameLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");

    ui->projectLocationLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");
    ui->defaultLanguageLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");
    ui->label->setStyleSheet("font-size: 14px; color: #333333;");
    ui->projectInfoLabel->setStyleSheet("font-size: 18px; color: #555555; padding-bottom: 10px;");
    ui->projectNameLineEdit->setStyleSheet("border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal; background-color: #ffffff; padding: 2px;");

    ui->browseButton->setStyleSheet(" border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal;background-color: #ffffff; padding: 2px;");
    ui->defaultLanguageComboBox->setStyleSheet("border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal; background-color: #ffffff; padding: 2px;");

    // ui->nextButton->setStyleSheet("border: 1px solid #cccccc; border-radius: 5px; padding: 3px 16px; background-color: #0F66DE; color: #ffffff; font-size: 14px; margin-inline:20px;");
    // ui->cancelButton->setStyleSheet("border: 1px solid #cccccc; border-radius: 5px; padding: 3px 16px; background-color: #f5f5f5; color: #333333; font-size: 14px;");
}

CreationAssistant::~CreationAssistant()
{
    delete ui;
}

std::string CreationAssistant::isValid(Project &project)
{
    std::string projectName = ui->projectNameLineEdit->text().toStdString();
    std::string projectPath = ui->browseButton->text().toStdString();
    // QString language = ui->defaultLanguageComboBox->currentText();

    if (projectName.empty()) {
        return "Give a name for the project";
    } else {
        project.setName(projectName);
    }
    if (projectPath == "Select path" || projectPath.empty()) {
        return "Select a path for your project";
    } else {
        project.setPath(projectPath);
    }

    return "";
}

void CreationAssistant::on_browseButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Project Location"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->browseButton->setText(dir);
    }
}
