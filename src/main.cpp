#include <QApplication>
#include "./components/stepper/stepper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Stepper stepper;
    stepper.show();
    return a.exec();
}
