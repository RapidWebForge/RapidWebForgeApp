#include "rapidwebforge.h"
#include "src/components/creation-assistant/creationassistant.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Crear instancia de la ventana diseñada
    CreationAssistant assistant;
    // Mostrar la ventana
    assistant.show();
    //RapidWebForge w;
    //w.show();
    return a.exec();
}
