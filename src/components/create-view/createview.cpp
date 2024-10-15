#include "createview.h"
#include "ui_createview.h"
#include <boost/algorithm/string.hpp>

CreateView::CreateView(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateView)
{
    ui->setupUi(this);
}

CreateView::~CreateView()
{
    delete ui;
}

void CreateView::on_createButton_clicked()
{
    std::string componentName = ui->viewNameLineEdit->text().toStdString();
    std::string path = ui->viewPathLineEdit->text().toStdString();
    // TODO: Ensure capitalize
    route.setComponent(componentName);
    route.setPath(boost::to_lower_copy(path));

    emit routeSaved(route);
}
