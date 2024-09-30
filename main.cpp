#include "rapidwebforge.h"
#include "src/components/creation-assistant/creationassistant.h"
#include "src/components/stepper/stepper.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Stepper stepper;
    stepper.show();
    return a.exec();
}
