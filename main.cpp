#include "rapidwebforge.h"
#include "src/components/creation-assistant/creationassistant.h"
#include "src/components/stepper/stepper.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Crear instancia de la ventana diseñada
    // CreationAssistant assistant;
    Stepper stepper;
    // Mostrar la ventana
    // assistant.show();
    stepper.show();
    //RapidWebForge w;
    //w.show();
    return a.exec();
}
