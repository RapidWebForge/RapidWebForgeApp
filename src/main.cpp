#include "./components/creation-assistant/creationassistant.h"
#include "./components/stepper/stepper.h"
#include "rapidwebforge.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Stepper stepper;
    stepper.show();
    return a.exec();
}
