#include "rapidwebforge.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    RapidWebForge w;
    w.show();
    return a.exec();
}
