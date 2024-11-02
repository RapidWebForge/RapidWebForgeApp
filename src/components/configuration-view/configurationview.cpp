#include "configurationview.h"
#include <QFileDialog>
#include <QMessageBox>
#include "ui_configurationview.h"

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
