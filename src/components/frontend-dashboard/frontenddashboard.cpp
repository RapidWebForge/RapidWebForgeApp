#include "frontenddashboard.h"
#include "ui_frontenddashboard.h"

FrontendDashboard::FrontendDashboard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrontendDashboard)
{
    ui->setupUi(this);
}

FrontendDashboard::~FrontendDashboard()
{
    delete ui;
}
