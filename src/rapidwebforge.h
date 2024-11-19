#ifndef RAPIDWEBFORGE_H
#define RAPIDWEBFORGE_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class RapidWebForge;
}
QT_END_NAMESPACE

class RapidWebForge : public QMainWindow
{
    Q_OBJECT

public:
    RapidWebForge(QWidget *parent = nullptr);
    ~RapidWebForge();

private:
    Ui::RapidWebForge *ui;
};
#endif // RAPIDWEBFORGE_H
