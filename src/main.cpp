#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include "./components/projects-panel/projectspanel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    // lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));        // Fondo de ventanas
    // lightPalette.setColor(QPalette::WindowText, Qt::black);                // Texto de ventanas
    // lightPalette.setColor(QPalette::Base, Qt::white);                      // Fondo de widgets
    // lightPalette.setColor(QPalette::AlternateBase, QColor(225, 225, 225)); // Fondo alterno
    // lightPalette.setColor(QPalette::ToolTipBase, Qt::white);               // Fondo de tooltips
    // lightPalette.setColor(QPalette::ToolTipText, Qt::black);               // Texto de tooltips
    // lightPalette.setColor(QPalette::Text, Qt::black);                      // Texto de widgets
    // lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));        // Fondo de botones
    // lightPalette.setColor(QPalette::ButtonText, Qt::black);                // Texto de botones
    // lightPalette.setColor(QPalette::BrightText, Qt::red);                  // Texto brillante

    // lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224)); // Color de selección
    // lightPalette.setColor(QPalette::HighlightedText, Qt::white);      // Texto seleccionado

    // a.setPalette(lightPalette);

    ProjectsPanel projectsPanel;
    projectsPanel.show();

    return a.exec();
}
