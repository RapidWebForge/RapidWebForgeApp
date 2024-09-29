#include "rapidwebforge.h"
#include "./ui_rapidwebforge.h"

RapidWebForge::RapidWebForge(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RapidWebForge)
{
    ui->setupUi(this);
}

RapidWebForge::~RapidWebForge()
{
    delete ui;
}
