#include "creationassistant.h"
#include "ui_creationassistant.h"
#include <QFileDialog>
#include <QMessageBox>

CreationAssistant::CreationAssistant(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreationAssistant)
{
    ui->setupUi(this);
    applyStylesCA(); // Aplicar todos los estilos
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
void CreationAssistant::applyStylesCA()
{
    QString generalStyle = "color: #333333; font-size: 14px; font-weight: normal;";
    QString inputStyle = "border: 1px solid #cccccc; font-size: 14px; border-radius: 5px; "
                         "font-weight: normal; background-color: #ffffff; padding: 4px 8px;";

    // Estilos para las etiquetas
    ui->projectNameLabel->setStyleSheet(generalStyle);
    ui->projectLocationLabel->setStyleSheet(generalStyle);
    ui->defaultLanguageLabel->setStyleSheet(generalStyle);
    ui->label->setStyleSheet(generalStyle);

    // Estilo específico para la etiqueta de información del proyecto
    ui->projectInfoLabel->setStyleSheet("font-size: 25px; color: #000000; padding-bottom: 0px; "
                                        "padding-left: 50px;");
    ui->titleLabel->setStyleSheet(
        "font-size: 35px; color: #27292A; padding-top: 10px; padding-left: 40px;");
    // Estilo para la línea de edición de texto
    ui->projectNameLineEdit->setStyleSheet(inputStyle);

    // Estilo para el botón de búsqueda
    ui->browseButton->setStyleSheet(inputStyle);

    // Estilo para el ComboBox
    ui->defaultLanguageComboBox->setStyleSheet(inputStyle);

    // Ajustar margenes y espaciamiento del layout principal (opcional)
    ui->verticalLayout->setContentsMargins(20, 0, 20, 0); // Ajusta los márgenes del layout
    ui->verticalLayout->setSpacing(10);                   // Ajusta el espaciado entre widgets
}
