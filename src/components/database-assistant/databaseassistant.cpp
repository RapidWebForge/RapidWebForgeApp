#include "databaseassistant.h"
#include "ui_databaseassistant.h"

DatabaseAssistant::DatabaseAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatabaseAssistant)
{
    ui->setupUi(this);
}

DatabaseAssistant::~DatabaseAssistant()
{
    delete ui;
}
