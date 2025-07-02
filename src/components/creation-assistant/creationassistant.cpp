#include "creationassistant.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include "../../core/project-manager/projectmanager.h"
#include "ui_creationassistant.h"

CreationAssistant::CreationAssistant(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CreationAssistant)
{
    ui->setupUi(this);
    applyStylesCA(); // Aplicar todos los estilos

    //Ocultar defaul language
    ui->defaultLanguageLabel->hide();
    ui->defaultLanguageComboBox->hide();
}

CreationAssistant::~CreationAssistant()
{
    delete ui;
}

std::string CreationAssistant::isValid(Project &project)
{
    ProjectManager projectManager;

    std::string projectName = ui->projectNameLineEdit->text().toStdString();
    std::string projectDescription = ui->descriptionPlainTextEdit->toPlainText().toStdString();
    std::string projectPath = ui->browseButton->text().toStdString();

    if (projectName.empty()) {
        return "Give a name for the project";
    } else {
        QRegularExpression invalidChars(R"([\\/:*?"<>|])");
        if (invalidChars.match(QString::fromStdString(projectName)).hasMatch())
            return "Invalid project name";
        if (projectManager.isProjectAvailable(projectName))
            project.setName(projectName);
        else
            return "A project with that name was already created";
    }
    if (!projectDescription.empty()) {
        project.setDescription(projectDescription);
    }
    if (projectPath == "Select path" || projectPath.empty()) {
        return "Select a path for your project";
    } else {
        project.setPath(projectPath + "/" + projectName);
    }

    // Validar si se debe crear un repositorio Git
    if (shouldCreateGitRepo()) {
        project.setVersions(true); // Habilitar versiones si se selecciona crear Git
    } else {
        project.setVersions(false);
    }
    return "";
}

void CreationAssistant::on_browseButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Select Project Location"));
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setOption(QFileDialog::DontResolveSymlinks, true);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    dialog.setDirectory(QDir::homePath());

    if (dialog.exec() == QDialog::Accepted) {
        QString dir = dialog.selectedFiles().first();
        if (!dir.isEmpty()) {
            ui->browseButton->setText(dir);
        }
    }
}

bool CreationAssistant::shouldCreateGitRepo()
{
    // Retorna verdadero si el checkbox está seleccionado
    return ui->createGitRepoCheckBox->isChecked();
}

void CreationAssistant::applyStylesCA()
{
    QString generalStyle = "color: #333333; font-size: 14px; font-weight: normal;";
    QString inputStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                         "font-weight: normal; background-color: #ffffff; padding: 4px 8px;";

    // Estilos para las etiquetas
    ui->projectNameLabel->setStyleSheet(generalStyle);
    ui->projectLocationLabel->setStyleSheet(generalStyle);
    ui->defaultLanguageLabel->setStyleSheet(generalStyle);
    ui->descriptionLabel->setStyleSheet(generalStyle);

    // Estilo específico para la etiqueta de información del proyecto
    ui->projectInfoLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                        "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");
    // Estilo para la línea de edición de texto
    ui->projectNameLineEdit->setStyleSheet(inputStyle);
    ui->descriptionPlainTextEdit->setStyleSheet(inputStyle);

    // Estilo para el botón de búsqueda
    ui->browseButton->setStyleSheet(inputStyle);

    // Estilo para el ComboBox
    ui->defaultLanguageComboBox->setStyleSheet(inputStyle);

    // Ajustar margenes y espaciamiento del layout principal (opcional)
    ui->verticalLayout->setContentsMargins(20, 0, 20, 0); // Ajusta los márgenes del layout
    ui->verticalLayout->setSpacing(10);                   // Ajusta el espaciado entre widgets
}
