#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeWidget>

class CustomTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit CustomTreeWidget(QWidget *parent = nullptr);

private:
    QRect dropIndicatorRect;        // Almacena el rectángulo del indicador
    bool showDropIndicator = false; // Controla si se debe mostrar el indicador

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

signals:
    void itemDropped(QTreeWidgetItem* parent, QTreeWidgetItem* item, int index);
};

#endif // CUSTOMTREEWIDGET_H
