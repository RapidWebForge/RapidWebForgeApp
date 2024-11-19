#include "projectpreview.h"
#include <QContextMenuEvent>
#include <QMenu>
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
    if (event->button() == Qt::LeftButton) {
        emit projectClicked(project);
    } else {
        // Deja que el clic derecho se maneje por el menú contextual
        QWidget::mousePressEvent(event);
    }
}

void ProjectPreview::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu(this); // Crea el menú en la pila
    QAction *deleteAction = contextMenu.addAction("Eliminar");

    connect(deleteAction, &QAction::triggered, [this]() { emit deleteRequested(project.getId()); });

    contextMenu.exec(event->globalPos()); // Ejecuta el menú contextual
}
