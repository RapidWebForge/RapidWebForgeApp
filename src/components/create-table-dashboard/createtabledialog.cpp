#include "createtabledialog.h"
#include <QFile>
#include "ui_createtabledialog.h"
#include <boost/algorithm/string.hpp>

CreateTableDialog::CreateTableDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateTableDialog)

{
    ui->setupUi(this);

    ui->addFieldButton->hide();

    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::close);

    applyStyles();
}

CreateTableDialog::~CreateTableDialog()
{
    delete ui;
}

void CreateTableDialog::applyStyles()
{
    QFile primaryButtonstyleFile(":/styles/primarybutton");
    if (primaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(primaryButtonstyleFile.readAll());
        ui->createButton->setStyleSheet(styleSheet);
    }
    QFile secondaryButtonstyleFile(":/styles/secondarybutton");
    if (secondaryButtonstyleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(secondaryButtonstyleFile.readAll());
        ui->cancelButton->setStyleSheet(styleSheet);
    }
}

void CreateTableDialog::on_createButton_clicked()
{
    std::string transactionName = ui->tableNameLineEdit->text().toStdString();
    // TODO: Ensure capitalize
    transaction.setName(transactionName);
    transaction.setNameConst(boost::to_lower_copy(transactionName));

    emit transactionSaved(transaction);
    // Log de la creación del modelo (concatenación directa)
    loggerJson.logAction("create-new-model", "Model create with name " + transactionName);
    // Limpiar el campo de texto después de crear la transacción
    ui->tableNameLineEdit->clear(); // Esto limpia el input de la tabla
    accept();
}
