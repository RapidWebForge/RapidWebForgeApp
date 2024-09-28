#ifndef FRONTENDASSISTANT_H
#define FRONTENDASSISTANT_H

#include <QDialog>
#include "../backend-assistant/backendassistant.h"  // Incluir la clase de la cuarta pantalla

namespace Ui {
class FrontendAssistant;
}

class FrontendAssistant : public QDialog
{
    Q_OBJECT

public:
    explicit FrontendAssistant(QWidget *parent = nullptr);
    ~FrontendAssistant();

private slots:
    void on_nextButton_clicked();   // Slot para el botón Next

private:
    Ui::FrontendAssistant *ui;
    BackendAssistant *backendAssistant;  // Puntero a la cuarta pantalla
};

#endif // FRONTENDASSISTANT_H
