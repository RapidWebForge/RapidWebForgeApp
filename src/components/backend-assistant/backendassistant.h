#ifndef BACKENDASSISTANT_H
#define BACKENDASSISTANT_H

#include <QDialog>

namespace Ui {
class BackendAssistant;
}

class BackendAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit BackendAssistant(QWidget *parent = nullptr);
    ~BackendAssistant();


private slots:
    void on_nextButton_clicked();   // Slot para el botón Next


private:
    Ui::BackendAssistant *ui;

    //QString projectName;           // Almacenar el nombre del proyecto
    //QString frontendFramework;     // Almacenar el framework frontend
};

#endif // BACKENDASSISTANT_H
