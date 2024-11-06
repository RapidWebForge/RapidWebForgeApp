#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

private:
    QTreeWidgetItem *draggedItem = nullptr;
    QPoint dragStartPosition;
    QString draggedItemText;

public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

signals:
    void itemDropped(QTreeWidgetItem *parentItem, QTreeWidgetItem *droppedItem, int dropIndex);

protected:
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // CUSTOMTREEWIDGET_H
