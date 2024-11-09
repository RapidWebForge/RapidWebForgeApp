#include "creationassistant.h"
#include <QFileDialog>
#include "ui_creationassistant.h"

CreationAssistant::CreationAssistant(QWidget *parent)
    : QDialog(parent)
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
    std::string projectName = ui->projectNameLineEdit->text().toStdString();
    std::string projectPath = ui->browseButton->text().toStdString();

    if (projectName.empty()) {
        return "Give a name for the project";
    } else {
        project.setName(projectName);
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
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select Project Location"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        ui->browseButton->setText(dir);
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

void CreationAssistant::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // Ignorar la tecla "Escape" globalmente o asignarle otra función
        event->ignore();
    } else {
        QWidget::keyPressEvent(event); // Ajusta esto si Stepper no hereda de QMainWindow
    }
}
