#include "configurationview.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include "ui_configurationview.h"

ConfigurationView::ConfigurationView(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigurationView)
    , configManager()
{
    ui->setupUi(this);

    // Valores por defecto
    const std::string defaultNgInxPath = "C:\\nginx-1.26.2\\nginx.exe";
    const std::string defaultNodePath = "C:\\Program Files\\nodejs\\node.exe";
    const std::string defaultBunPath = "C:\\Users\\{{ user }}\\.bun\\bin\\bun.exe";
    const std::string defaultMysqlPath
        = "C:\\Program Files\\MySQL\\MySQL Server 9.0\\bin\\mysql.exe";

    bool status = configManager.getConfiguration().getStatus();
    ui->saveButton->setEnabled(status);

    // Obtener los valores de la configuración
    Configuration conf = configManager.getConfiguration();

    std::string nginxPath = conf.getNgInxPath().empty() ? defaultNgInxPath : conf.getNgInxPath();
    std::string nodePath = conf.getnodePath().empty() ? defaultNodePath : conf.getnodePath();
    std::string bunPath = conf.getBunPath().empty() ? defaultBunPath : conf.getBunPath();
    std::string mysqlPath = conf.getMysqlPath().empty() ? defaultMysqlPath : conf.getMysqlPath();

    if (!nginxPath.empty()) {
        QString path = QString::fromStdString(nginxPath);
        ui->ngInxPathLineEdit->setText(path);
        ui->ngInxPathButton->setText(path);
    }
    if (!nodePath.empty()) {
        QString path = QString::fromStdString(nodePath);
        ui->nodePathLineEdit->setText(path);
        ui->nodePathButton->setText(path);
    }
    if (!bunPath.empty()) {
        QString path = QString::fromStdString(bunPath);
        ui->bunPathLineEdit->setText(path);
        ui->bunPathButton->setText(path);
    }
    if (!mysqlPath.empty()) {
        QString path = QString::fromStdString(mysqlPath);
        ui->mysqlPathLineEdit->setText(path);
        ui->mysqlPathButton->setText(path);
    }

    connect(ui->ngInxPathLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        ui->ngInxPathButton->setText(text.isEmpty() ? "Path" : text);
    });

    connect(ui->nodePathLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        ui->nodePathButton->setText(text.isEmpty() ? "Path" : text);
    });

    connect(ui->bunPathLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        ui->bunPathButton->setText(text.isEmpty() ? "Path" : text);
    });

    connect(ui->mysqlPathLineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        ui->mysqlPathButton->setText(text.isEmpty() ? "Path" : text);
    });
}

ConfigurationView::~ConfigurationView()
{
    delete ui;
}

void ConfigurationView::selectDirectoryAndSetUI(QPushButton *button,
                                                QLineEdit *lineEdit,
                                                const QString &title)
{
    QFileDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setOption(QFileDialog::DontResolveSymlinks, true);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setDirectory(QDir::homePath());

    if (dialog.exec() == QDialog::Accepted) {
        QString dir = dialog.selectedFiles().first();
        if (!dir.isEmpty()) {
            button->setText(dir);
            lineEdit->setText(dir);
        }
    }
}

void ConfigurationView::on_ngInxPathButton_clicked()
{
    selectDirectoryAndSetUI(ui->ngInxPathButton, ui->ngInxPathLineEdit, tr("Select NgInx Location"));
}

void ConfigurationView::on_nodePathButton_clicked()
{
    selectDirectoryAndSetUI(ui->nodePathButton, ui->nodePathLineEdit, tr("Select Node Location"));
}

void ConfigurationView::on_bunPathButton_clicked()
{
    selectDirectoryAndSetUI(ui->bunPathButton, ui->bunPathLineEdit, tr("Select Bun Location"));
}

void ConfigurationView::on_mysqlPathButton_clicked()
{
    selectDirectoryAndSetUI(ui->mysqlPathButton, ui->mysqlPathLineEdit, tr("Select MySQL Location"));
}

void ConfigurationView::on_testButton_clicked()
{
    ui->testButton->setEnabled(false); // Desactiva temporalmente el botón para evitar doble clic

    QStringList paths = {ui->ngInxPathLineEdit->text(),
                         ui->nodePathLineEdit->text(),
                         ui->bunPathLineEdit->text(),
                         ui->mysqlPathLineEdit->text()};
    QStringList commands = {
        "-version",  // para Nginx
        "--version", // para Node.js
        "--version", // para Bun
        "--version"  // para MySQL
    };
    QStringList names = {"Nginx", "Node.js", "Bun", "MySQL"};
    std::vector<std::string> invalidPaths;
    bool allPathsValid = true;

    for (int i = 0; i < paths.size(); ++i) {
        if (!paths[i].isEmpty()
            && !checkPathValid(paths[i].toStdString(), commands[i].toStdString())) {
            invalidPaths.push_back(names[i].toStdString());
            allPathsValid = false;
        }
    }

    Configuration conf = configManager.getConfiguration();

    if (!allPathsValid) {
        conf.setStatus(false);

        configManager.setConfiguration(conf);

        ui->saveButton->setEnabled(false); // Deshabilita el botón de guardar

        std::string message = "The following paths are invalid:\n";
        for (const auto &path : invalidPaths) {
            message += "- " + path + "\n";
        }
        QMessageBox::critical(this, "Invalid Paths", QString::fromStdString(message));
    } else {
        conf.setStatus(true);

        configManager.setConfiguration(conf);

        ui->saveButton->setEnabled(true); // Habilita el botón de guardar

        QMessageBox::information(this, "Success", "All paths are valid.");
    }

    ui->testButton->setEnabled(true); // Vuelve a habilitar el botón de probar
}

void ConfigurationView::on_saveButton_clicked()
{
    QString nginxPath = ui->ngInxPathLineEdit->text();
    QString nodePath = ui->nodePathLineEdit->text();
    QString bunPath = ui->bunPathLineEdit->text();
    QString mysqlPath = ui->mysqlPathLineEdit->text();
    Configuration conf = configManager.getConfiguration();

    if (ui->ngInxPathButton->text() == "Path") {
        QMessageBox::warning(this, "Invalid Operation", "You must to select a nginx path to save");
        return;
    }
    if (configManager.getConfiguration().getNgInxPath() != nginxPath.toStdString())
        conf.setNgInxPath(nginxPath.toStdString());
    if (ui->nodePathButton->text() == "Path") {
        QMessageBox::warning(this, "Invalid Operation", "You must to select a node path to save");
        return;
    }
    if (configManager.getConfiguration().getnodePath() != nodePath.toStdString())
        conf.setnodePath(nodePath.toStdString());
    if (ui->bunPathButton->text() == "Path") {
        QMessageBox::warning(this, "Invalid Operation", "You must to select a bun path to save");
        return;
    }
    if (configManager.getConfiguration().getBunPath() != bunPath.toStdString())
        conf.setBunPath(bunPath.toStdString());
    if (ui->mysqlPathButton->text() == "Path") {
        QMessageBox::warning(this, "Invalid Operation", "You must to select a mysql path to save");
        return;
    }
    if (configManager.getConfiguration().getMysqlPath() != mysqlPath.toStdString())
        conf.setMysqlPath(mysqlPath.toStdString());

    configManager.setConfiguration(conf);

    QMessageBox::information(this, "Successfully", "Path set successfully");
}

bool ConfigurationView::checkPathValid(const std::string &path, const std::string &versionFlag)
{
    // Usamos QProcess::execute para simplificar
    const QString program = QString::fromStdString(path);
    const QStringList args = {QString::fromStdString(versionFlag)};

    // QProcess::execute arranca el programa, espera a que termine y devuelve el código de salida
    int exitCode = QProcess::execute(program, args);
    return (exitCode == 0);
}
