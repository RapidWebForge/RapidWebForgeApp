#include "manageversion.h"
#include "ui_manageversion.h"

ManageVersion::ManageVersion(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManageVersion)
{
    ui->setupUi(this);
}

ManageVersion::~ManageVersion()
{
    delete ui;
}
