#include "creationassistant.h"
#include "ui_creationassistant.h"
#include <QFileDialog>
#include <QMessageBox>

CreationAssistant::CreationAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreationAssistant)
{
    ui->setupUi(this);
    ui->projectInfoGroupBox->setStyleSheet("border: none;");

    ui->projectNameLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");

    ui->projectLocationLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");
    ui->defaultLanguageLabel->setStyleSheet("color: #333333; font-size: 14px; font-weight: normal;");
    ui->label->setStyleSheet("font-size: 14px; color: #333333;");
    ui->projectInfoLabel->setStyleSheet("font-size: 18px; color: #555555; padding-bottom: 10px;");
    ui->projectNameLineEdit->setStyleSheet("border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal; background-color: #ffffff; padding: 2px;");

    ui->browseButton->setStyleSheet(" border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal;background-color: #ffffff; padding: 2px;");
    ui->defaultLanguageComboBox->setStyleSheet("border: 1px solid #cccccc; font-size: 14; border-radius: 5px; font-weight: normal; background-color: #ffffff; padding: 2px;");


    ui->nextButton->setStyleSheet("border: 1px solid #cccccc; border-radius: 5px; padding: 3px 16px; background-color: #0F66DE; color: #ffffff; font-size: 14px; margin-inline:20px;");
    ui->cancelButton->setStyleSheet("border: 1px solid #cccccc; border-radius: 5px; padding: 3px 16px; background-color: #f5f5f5; color: #333333; font-size: 14px;");

    // Connect buttons to their respective slots
    connect(ui->nextButton, &QPushButton::clicked, this, &CreationAssistant::handleNextButton);
    connect(ui->browseButton, &QPushButton::clicked, this, &CreationAssistant::on_browseButton_clicked);
}

CreationAssistant::~CreationAssistant()
{
    delete ui;
}

void CreationAssistant::handleNextButton()
{
    QString projectName = ui->projectNameLineEdit->text();
    QString projectLocation = ui->browseButton->text();
    QString language = ui->defaultLanguageComboBox->currentText();

    if (projectName.isEmpty() || projectLocation.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all the fields.");
        return;
    }

    QMessageBox::information(this, "Project Created", "Project created successfully!\nName: " + projectName +
                                                          "\nLocation: " + projectLocation + "\nLanguage: " + language);
}

void CreationAssistant::on_browseButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Project Location"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->browseButton->setText(dir);
    }
}
