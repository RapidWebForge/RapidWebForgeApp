#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

signals:
    void itemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem);

protected:
    void dropEvent(QDropEvent *event) override;
};

#endif // CUSTOMTREEWIDGET_H
