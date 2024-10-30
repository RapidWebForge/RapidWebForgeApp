#include "createversion.h"
#include "ui_createversion.h"

CreateVersion::CreateVersion(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateVersion)
{
    ui->setupUi(this);
}

CreateVersion::~CreateVersion()
{
    delete ui;
}
