#include "createview.h"
#include <QMessageBox>
#include "ui_createview.h"
#include <boost/algorithm/string.hpp>

CreateView::CreateView(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateView)
{
    ui->setupUi(this);
    applyStyles();
}

CreateView::~CreateView()
{
    delete ui;
}

void CreateView::applyStyles()
{
    ui->createButton->setStyleSheet("QPushButton {"
                                    "   background-color: #0F66DE;"
                                    "   color: white;"
                                    "   border-radius: 5px;"
                                    "   padding: 4px 30px;"
                                    "   font-size: 14px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "   background-color: #0056b3;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "   background-color: #004494;"
                                    "}");

    ui->cancelButton->setStyleSheet("QPushButton {"
                                    "   color: black;"
                                    "   border-radius: 5px;"
                                    "   padding: 4px 30px;"
                                    "   font-size: 14px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "   background-color: #eaeaea;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "   background-color: #004494;"
                                    "}");
}

void CreateView::on_createButton_clicked()
{
    std::string componentName = ui->viewNameLineEdit->text().toStdString();
    std::string path = ui->viewPathLineEdit->text().toStdString();
    // TODO: Ensure capitalize
    route.setComponent(componentName);
    route.setPath(boost::to_lower_copy(path));

    emit routeSaved(route);

    if (!componentName.empty() && !path.empty())
        accept();
    else
        QMessageBox::warning(this, "Warning", "Fill all the fields to create");
}

void CreateView::on_cancelButton_clicked()
{
    accept();
}
