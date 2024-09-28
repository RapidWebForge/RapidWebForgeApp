#include "creationassistant.h"
#include "ui_creationassistant.h"
#include <QFileDialog>
#include <QMessageBox>

CreationAssistant::CreationAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreationAssistant)
{
    ui->setupUi(this);

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
    QString projectLocation = ui->projectLocationLineEdit->text();
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
        ui->projectLocationLineEdit->setText(dir);
    }
}
