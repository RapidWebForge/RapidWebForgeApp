#include "backendassistant.h"
#include "ui_backendassistant.h"

BackendAssistant::BackendAssistant(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackendAssistant)
{
    ui->setupUi(this);
}

BackendAssistant::~BackendAssistant()
{
    delete ui;
}
