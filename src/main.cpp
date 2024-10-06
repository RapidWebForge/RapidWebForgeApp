#include <QApplication>
#include "./components/stepper-dashboard/stepperdashboard.h"
#include "./components/stepper/stepper.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Stepper stepper;
    //stepper.show();
    StepperDashboard stprDashboard;
    stprDashboard.show();
    return a.exec();
}
