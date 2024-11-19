#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void itemDropped(QTreeWidgetItem* parent, QTreeWidgetItem* item, int index);
};

#endif // CUSTOMTREEWIDGET_H
