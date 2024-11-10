#include "configurationview.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_configurationview.h"
#include <boost/process.hpp> // Incluir Boost.Process

ConfigurationView::ConfigurationView(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConfigurationView)
    , configManager()
{
    ui->setupUi(this);

    bool status = configManager.getConfiguration().getStatus();
    ui->saveButton->setEnabled(status);

    std::string nginxPath = configManager.getConfiguration().getNgInxPath();
    std::string nodePath = configManager.getConfiguration().getnodePath();
    std::string bunPath = configManager.getConfiguration().getBunPath();
    std::string mysqlPath = configManager.getConfiguration().getMysqlPath();

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

void ConfigurationView::on_ngInxPathButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    tr("Select NgInx Location"),
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        // Actualiza tanto el botón como el QLineEdit
        ui->ngInxPathButton->setText(dir);
        ui->ngInxPathLineEdit->setText(dir);
    }
}
void ConfigurationView::on_testButton_clicked()
{
    ui->testButton->setEnabled(false); // Desactiva temporalmente el botón para evitar doble clic

    QStringList paths = {ui->ngInxPathLineEdit->text(),
                         ui->nodePathLineEdit->text(),
                         ui->bunPathLineEdit->text(),
                         ui->mysqlPathLineEdit->text()};
    QStringList commands = {
        " -version",  // para Nginx
        " --version", // para Node.js
        " --version", // para Bun
        " --version"  // para MySQL
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

    if (!allPathsValid) {
        configManager.getConfiguration().setStatus(false);

        configManager.setConfiguration(configManager.getConfiguration());

        ui->saveButton->setEnabled(false); // Deshabilita el botón de guardar

        std::string message = "The following paths are invalid:\n";
        for (const auto &path : invalidPaths) {
            message += "- " + path + "\n";
        }
        QMessageBox::critical(this, "Invalid Paths", QString::fromStdString(message));
    } else {
        configManager.getConfiguration().setStatus(true);

        configManager.setConfiguration(configManager.getConfiguration());

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
    namespace bp = boost::process;
    try {
        bp::ipstream is;
        bp::child c(path + versionFlag,
                    bp::std_out > is,
                    bp::std_err > bp::null); // Redirigir errores a null

        c.wait();
        return c.exit_code() == 0;
    } catch (...) {
        return false; // Si ocurre una excepción, retornar falso
    }
}
