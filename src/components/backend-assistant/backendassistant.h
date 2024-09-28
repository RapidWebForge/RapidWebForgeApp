#ifndef BACKENDASSISTANT_H
#define BACKENDASSISTANT_H

#include <QDialog>
#include "../summary-assistant/summaryassistant.h"  // Incluir la clase de la pantalla de resumen

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
    SummaryAssistant *summaryAssistant;  // Puntero a la pantalla de resumen

    //QString projectName;           // Almacenar el nombre del proyecto
    //QString frontendFramework;     // Almacenar el framework frontend
};

#endif // BACKENDASSISTANT_H
