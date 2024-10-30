#include "versionhistory.h"
#include "ui_versionhistory.h"

VersionHistory::VersionHistory(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::VersionHistory)
{
    ui->setupUi(this);
}

VersionHistory::~VersionHistory()
{
    delete ui;
}
