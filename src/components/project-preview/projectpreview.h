#ifndef PROJECTPREVIEW_H
#define PROJECTPREVIEW_H

#include <QWidget>
#include "../../models/project/project.h"

namespace Ui {
class ProjectPreview;
}

class ProjectPreview : public QWidget
{
    Q_OBJECT

public:
    ProjectPreview(QWidget *parent, const Project &project);
    ~ProjectPreview();

signals:
    void projectClicked(const Project &project);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::ProjectPreview *ui;
    Project project;
};

#endif // PROJECTPREVIEW_H
