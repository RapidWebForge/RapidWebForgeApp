#include <QApplication>
#include "./components/projects-panel/projectspanel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ProjectsPanel projectsPanel;
    projectsPanel.show();
    return a.exec();
}
