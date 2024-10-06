#include "backenddashboard.h"
#include "ui_backenddashboard.h"

BackendDashboard::BackendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackendDashboard)
{
    ui->setupUi(this);
}

BackendDashboard::~BackendDashboard()
{
    delete ui;
}
