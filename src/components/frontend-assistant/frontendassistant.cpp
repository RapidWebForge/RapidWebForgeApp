#include "frontendassistant.h"
#include "ui_frontendassistant.h"

FrontendAssistant::FrontendAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FrontendAssistant)
{
    ui->setupUi(this);
}

FrontendAssistant::~FrontendAssistant()
{
    delete ui;
}
