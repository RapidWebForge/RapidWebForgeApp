#include <QApplication>
#include "./components/stepper/stepper.h"
#include <fmt/core.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Stepper stepper;
    stepper.show();
    return a.exec();
}
