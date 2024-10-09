#include "projectpreview.h"
#include "ui_projectpreview.h"

ProjectPreview::ProjectPreview(QWidget *parent, const Project &project)
    : QWidget(parent)
    , ui(new Ui::ProjectPreview)
    , project(project)
{
    ui->setupUi(this);

    ui->labelProject->setText(QString::fromStdString(this->project.getName()));
}

ProjectPreview::~ProjectPreview()
{
    delete ui;
}

void ProjectPreview::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    // Emit the signal when the widget is clicked
    emit projectClicked(project);
}
