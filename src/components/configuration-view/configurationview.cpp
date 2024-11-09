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

void ConfigurationView::on_saveButton_clicked()
{
    QString nginxPath = ui->ngInxPathLineEdit->text();
    QString nodePath = ui->nodePathLineEdit->text();
    QString bunPath = ui->bunPathLineEdit->text();
    QString mysqlPath = ui->mysqlPathLineEdit->text();

    // Crear un comando compuesto para verificar todas las rutas en una sola ventana de cmd
    std::string composedCommand = "cmd /C \"";

    if (!nginxPath.isEmpty()) {
        composedCommand += "\"" + nginxPath.toStdString()
                           + "\" -version || echo nginx_path_invalid && ";
    }
    if (!nodePath.isEmpty()) {
        composedCommand += "\"" + nodePath.toStdString()
                           + "\" --version || echo node_path_invalid && ";
    }
    if (!bunPath.isEmpty()) {
        composedCommand += "\"" + bunPath.toStdString()
                           + "\" --version || echo bun_path_invalid && ";
    }
    if (!mysqlPath.isEmpty()) {
        composedCommand += "\"" + mysqlPath.toStdString()
                           + "\" --version || echo mysql_path_invalid && ";
    }

    // Remover el último `&& ` y cerrar el comando
    composedCommand = composedCommand.substr(0, composedCommand.size() - 4) + "\"";

    namespace bp = boost::process;
    bp::ipstream is; // Stream para capturar la salida
    bp::child c(composedCommand,
                bp::std_out > is,
                bp::std_err > bp::null); // Redirigir errores para evitar ventanas de error

    std::vector<std::string> invalidPaths;
    std::string line;
    while (std::getline(is, line)) {
        if (line.find("nginx_path_invalid") != std::string::npos) {
            invalidPaths.push_back("Nginx");
        } else if (line.find("node_path_invalid") != std::string::npos) {
            invalidPaths.push_back("Node.js");
        } else if (line.find("bun_path_invalid") != std::string::npos) {
            invalidPaths.push_back("Bun");
        } else if (line.find("mysql_path_invalid") != std::string::npos) {
            invalidPaths.push_back("MySQL");
        }
    }

    c.wait(); // Esperar a que el proceso termine

    // Verificar si hubo rutas inválidas y mostrar un mensaje
    if (!invalidPaths.empty()) {
        std::string message = "The following paths are invalid:\n";
        for (const auto &path : invalidPaths) {
            message += "- " + path + "\n";
        }
        QMessageBox::critical(this, "Invalid Paths", QString::fromStdString(message));
        return;
    }

    // Guardar la configuración si todas las rutas son válidas
    Configuration conf = configManager.getConfiguration();

    if (configManager.getConfiguration().getNgInxPath() != nginxPath.toStdString())
        conf.setNgInxPath(nginxPath.toStdString());
    if (configManager.getConfiguration().getnodePath() != nodePath.toStdString())
        conf.setnodePath(nodePath.toStdString());
    if (configManager.getConfiguration().getBunPath() != bunPath.toStdString())
        conf.setBunPath(bunPath.toStdString());
    if (configManager.getConfiguration().getMysqlPath() != mysqlPath.toStdString())
        conf.setMysqlPath(mysqlPath.toStdString());

    configManager.setConfiguration(conf);

    QMessageBox::information(this, "Success", "Paths have been saved successfully.");
}
// Función auxiliar para verificar cada path
bool ConfigurationView::checkPathValid(const std::string &path, const std::string &versionFlag)
{
    namespace bp = boost::process;
    try {
        // Ejecutar el comando de prueba
        std::string command = path + versionFlag;
        bp::ipstream is; // Stream para capturar la salida
        bp::child c(command,
                    bp::std_out > is,
                    bp::std_err > bp::null); // Redirigir errores para evitar ventanas

        c.wait();                  // Esperar a que el proceso termine
        return c.exit_code() == 0; // Retornar verdadero si el comando fue exitoso
    } catch (...) {
        return false; // Si ocurre una excepción, retornar falso
    }
}
