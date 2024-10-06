#include "addfielddialog.h"
#include "ui_addfielddialog.h"

AddFieldDialog::AddFieldDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFieldDialog)
{
    ui->setupUi(this);
}

AddFieldDialog::~AddFieldDialog()
{
    delete ui;
}
